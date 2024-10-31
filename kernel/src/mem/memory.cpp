#include "memory.h"

#include <stdio.h>
#include <units.h>

namespace memory {

    PageAllocator allocator;
    VMM vmm;
    KernelHeap kheap;
    bamboo_mmap_t* map;

    /* This function should be called AFTER doing ACPI things, as the memory used for
        some ACPI tables is reclaimed (as it is marked as EfiACPIReclaimMemory) */
    void parse_memory_map(bamboo_mmap_t* mmap) {
        map = mmap;

        efi_memory_descriptor_t* ent = mmap->map;
        do {
            ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + mmap->desc_size);
        } while((uint8_t*) ent < (uint8_t*) mmap->map + mmap->map_size);

        uint64_t memory_size = get_usable_memory_size();

        io::print_drv_name("PMM");
        io::printf("Total usable memory: %u pages, or %uMB\n", memory_size, b_to_mb(memory_size * 0x1000));
    }

    /* Initialises the PageAllocator */
    void allocator_init() {
        allocator = PageAllocator(map);
    }

    /* Initialises the kernel's virtual memory manager */
    void vmm_init(bamboo_boot_info_t* boot_info) {
        vmm = VMM();

        io::print_drv_name("VMM");
        io::printf("Identity mapping memory + mapping kernel to higher half...");

        /* Identity map all of our memory */

        identity_map_memory();

        /* Making sure our framebuffer is still mapped to its original location */
        void* framebuffer_base = boot_info->framebuffer->base_address;
        uint64_t framebuffer_size = (uint64_t) boot_info->framebuffer->buffer_size + 0x1000;

        allocator.lock_pages(boot_info->framebuffer->base_address, framebuffer_size / 0x1000 + 1);
        
        for(uint64_t i = (uint64_t)framebuffer_base; i < (uint64_t)framebuffer_base + framebuffer_size; i += 0x1000) {
            vmm.map_memory((void*) i, (void*) i);
        }

        /* Map our kernel up to 0xFFFF800000000000 */
        
        uint64_t map_to = KERNEL_START;
        for(uint64_t i = _kernel_phys_start; i < boot_info->kernel_phys_end; i += 0x1000) {
            vmm.map_memory((void*) i, (void*) map_to);
            map_to += 0x1000;
        }
                
        set_vmm(&vmm);

        io::clear_current_line();

        io::print_drv_name("VMM");
        io::printf("Initialised VMM, higher-half kernel mapping complete to 0x%X\n", KERNEL_START);
    }

    /* Initialises the kernel's heap */
    void kheap_init() {
        kheap = KernelHeap((void*) KERNEL_HEAP_START);
    }

    void identity_map_memory() {
        efi_memory_descriptor_t* ent = map->map;
        
        do {
            for(int i = 0; i < ent->number_of_pages * 0x1000; i += 0x1000) {
                vmm.map_memory((void*)(ent->physical_start + i), (void*)(ent->physical_start + i));
            }
            ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + map->desc_size);
        } while((uint8_t*) ent < (uint8_t*) map->map + map->map_size);
    }

    uint64_t get_usable_memory_size() {
        uint64_t size = 0;
        efi_memory_descriptor_t* ent = map->map;
        
        do {
            if(is_region_usable(ent)) {
                size += ent->number_of_pages;
            }
            ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + map->desc_size);
        } while((uint8_t*) ent < (uint8_t*) map->map + map->map_size);

        return size; // In pages
    }

    uint64_t get_total_memory_size() {
        uint64_t size = 0;
        efi_memory_descriptor_t* ent = map->map;
        
        do {
            size += ent->number_of_pages;
            ent = (efi_memory_descriptor_t*)(((uint8_t*) ent) + map->desc_size);
        } while((uint8_t*) ent < (uint8_t*) map->map + map->map_size);

        return size; // In pages
    }

    bool is_region_usable(efi_memory_descriptor_t* ent) {
        return ent->type == EfiConventionalMemory || ent->type == EfiBootServicesCode || ent->type == EfiBootServicesData
                || ent->type == EfiLoaderCode || ent->type == EfiLoaderData;
    }

    /* Takes in usable page number 'upn' and returns the address of the nth usable page
       Note: function returns 0 if the page number is out of bounds / page doesn't exist */
    void* upn_to_address(uint64_t upn) {
        uint64_t current_page = 0;
        efi_memory_descriptor_t* region = map->map;

        do {
            if(is_region_usable(region)) { // Skip non-free regions

                if(current_page + region->number_of_pages > upn) { // The n'th free page is in this region
                    uint64_t offset = upn - current_page; // In pages
                    return (void*)((uint64_t)(region->physical_start) + (offset * 0x1000));
                }

                current_page += region->number_of_pages;
            }

            region = (efi_memory_descriptor_t*)(((uint8_t*) region) + map->desc_size);
        } while((uint8_t*) region < (uint8_t*) map->map + map->map_size);

        return 0;
    }

    /* Takes an address of a usable page and finds the index of that page in the list of usable pages (opposite of 'nth_usable_page')
       Note: function returns 0 if the address is invalid */
    uint64_t address_to_upn(void* address) {
        if((uint64_t) address % 0x1000 != 0) return 0;

        uint64_t current_page = 0;
        efi_memory_descriptor_t* region = map->map;

        do {
            if(is_region_usable(region)) { // Skip non-free regions

                if((uint64_t) region->physical_start <= (uint64_t) address && (uint64_t) region->physical_start + (region->number_of_pages * 0x1000) < (uint64_t) address) {
                    return 0;
                }

                if((uint64_t) region->physical_start + (region->number_of_pages * 0x1000) > (uint64_t) address) {
                    uint64_t offset = (uint64_t) address - (uint64_t) region->physical_start; // In bytes
                    if(offset % 0x1000 != 0) return 0;

                    return current_page + (offset / 0x1000);
                }

                current_page += region->number_of_pages;
            }

            region = (efi_memory_descriptor_t*)(((uint8_t*) region) + map->desc_size);
        } while((uint8_t*) region < (uint8_t*) map->map + map->map_size);

        return 0;
    }

    void set_vmm(VMM* new_vmm) {
        asm volatile ("mov %0, %%cr3" : : "r" (((void*) new_vmm->get_pml4())));
    }

    PageAllocator* get_allocator() {
        return &allocator;
    }

    VMM* get_vmm() {
        return &vmm;
    }

    KernelHeap* get_kheap() {
        return &kheap;
    }
}
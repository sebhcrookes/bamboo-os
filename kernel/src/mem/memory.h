#pragma once

#include <stdint.h>

#include "../boot_structs.h"
#include "efi_memory.h"
#include "allocator.h"
#include "vmm.h"
#include "kheap.h"

#include "../kernel.h"

/* Note: UPN is short for usable page number - that is, 0 references
   the first usable page, 1 references the second usable page etc. */

#define FRAMEBUFFER_ADDR 0xFFFF000000000000
#define KERNEL_START 0xFFFF800000000000
#define KERNEL_HEAP_START 0xFFFF900000000000

namespace memory {
    void parse_memory_map(bamboo_mmap_t* mmap);
    void allocator_init();
    void vmm_init(bamboo_boot_info_t* boot_info);
    void kheap_init();

    void identity_map_memory();

    uint64_t get_usable_memory_size();
    uint64_t get_total_memory_size();

    bool is_region_usable(efi_memory_descriptor_t* ent);
    void* upn_to_address(uint64_t upn);
    uint64_t address_to_upn(void* address);

    void set_vmm(VMM* new_vmm);

    PageAllocator* get_allocator();
    VMM* get_vmm();
    KernelHeap* get_kheap();
}
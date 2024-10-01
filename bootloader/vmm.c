#include "vmm.h"

#include <uefi.h>

#include <string.h>

#include "memory.h"

void virtual_to_indices(uint64_t virtual_address, uint64_t* PDP_i, uint64_t* PD_i, uint64_t* PT_i, uint64_t* P_i) {
    // Hi, Felix put this here :)
    *P_i    = (virtual_address >>= 12) & 0x1FF;
    *PT_i   = (virtual_address >>= 9)  & 0x1FF;
    *PD_i   = (virtual_address >>= 9)  & 0x1FF;
    *PDP_i  = (virtual_address >>= 9)  & 0x1FF;
}

void map_memory(page_table_t* pml4, void* phys, void* virt) {
    uint64_t PDP_i;
    uint64_t PD_i;
    uint64_t PT_i;
    uint64_t P_i;

    // Convert the virtual address into the indices we need
    virtual_to_indices((uint64_t) virt, &PDP_i, &PD_i, &PT_i, &P_i);
    
    pde_t PDE;

    PDE = pml4->entries[PDP_i];
    
    /* Getting the PDP (page directory pointer) */
    page_table_t* PDP;
    if (!get_flag(&PDE, PRESENT)) { // If the PDE is not present then we need to create a new one
        efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*) &PDP);
        memset(PDP, 0, 0x1000);

        set_address(&PDE, (uint64_t) PDP >> 12); // Set the PDP's address to the address which we allocated

        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);

        // Finally, set the entry in the PML4 to this new PDP
        pml4->entries[PDP_i] = PDE;
    } else {
        PDP = (page_table_t*)((uint64_t) get_address(&PDE) << 12);
    }
    
    PDE = PDP->entries[PD_i];

    /* Getting the PD (page directory) */
    page_table_t* PD;
    if (!get_flag(&PDE, PRESENT)) {
        efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*) &PD);
        memset(PD, 0, 0x1000);

        set_address(&PDE, (uint64_t) PD >> 12);

        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);
        
        PDP->entries[PD_i] = PDE;
    } else {
        PD = (page_table_t*)((uint64_t) get_address(&PDE) << 12);
    }

    PDE = PD->entries[PT_i];

    /* Getting the PT (page table) */
    page_table_t* PT;
    if (!get_flag(&PDE, PRESENT)) {
        efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*) &PT);
        memset(PT, 0, 0x1000);

        set_address(&PDE, (uint64_t) PT >> 12);

        set_flag(&PDE, PRESENT, true);
        set_flag(&PDE, READ_WRITE, true);
        
        PD->entries[PT_i] = PDE;
    } else {
        PT = (page_table_t*)((uint64_t) get_address(&PDE) << 12);
    }

    PDE = PT->entries[P_i];

    set_address(&PDE, (uint64_t) phys >> 12);
    set_flag(&PDE, PRESENT, true);
    set_flag(&PDE, READ_WRITE, true);
    PT->entries[P_i] = PDE;
}

void set_flag(pde_t* pde, int flag, bool enabled) {
    uint64_t bit_selector = (uint64_t) 1 << flag;
    pde->value &= ~bit_selector;
    if(enabled) {
        pde->value |= bit_selector;
    }
}

bool get_flag(pde_t* pde, int flag) {
    uint64_t bit_selector = (uint64_t) 1 << flag;
    return pde->value & bit_selector > 0 ? true : false;
}

uint64_t get_address(pde_t* pde){
    return (pde->value & 0x000FFFFFFFFFF000) >> 12;
}

void set_address(pde_t* pde, uint64_t address) {
    address &= 0x000000FFFFFFFFFF;
    pde->value &= 0xFFF0000000000FFF;
    pde->value |= (address << 12);
}
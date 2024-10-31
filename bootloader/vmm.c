#include "vmm.h"

#include <uefi.h>

#include <string.h>

#include "memory.h"

void map_memory(page_table_t* pml4, void* phys, void* virt) {    
    /* Finding the indices from the virtual address */
    uint64_t indices[4] = { 0, 0, 0, 0 }; // Holds indices in order: PDP, PD, PT, P

    uint64_t curr = (uint64_t) virt;
    for(int i = 0; i < 4; i++) {
        if(i == 0) curr >>= 12;
        else curr >>= 9;

        indices[3 - i] = curr & 0x1FF;
    }
    
    /* Traversing the tables, creating new ones if necessary */
    pde_t PDE = pml4->entries[indices[0]];

    page_table_t* last_table = pml4;
    page_table_t* table;
    for(int i = 0; i < 3; i++) {
        if (!get_flag(&PDE, PRESENT)) { // If the table is not present then we need to create a new one
            /* Allocate space for a table */
            efi_status_t status = BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (efi_physical_address_t*) &table);
            memset(table, 0, 0x1000);

            // Setting the PDE's address and flags
            set_address(&PDE, (uint64_t) table >> 12);
            set_flag(&PDE, PRESENT, true);
            set_flag(&PDE, READ_WRITE, true);

            // Finally, set the entry in the PML4 to this new PDP
            last_table->entries[indices[i]] = PDE;
        } else {
            table = (page_table_t*)((uint64_t) get_address(&PDE) << 12);
        }

        PDE = table->entries[indices[i + 1]];

        last_table = table;
    }

    set_address(&PDE, (uint64_t) phys >> 12);
    set_flag(&PDE, PRESENT, true);
    set_flag(&PDE, READ_WRITE, true);
    table->entries[indices[3]] = PDE;
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
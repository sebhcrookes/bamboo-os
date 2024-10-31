#include "vmm.h"

#include <string.h>
#include <stdio.h>

#include "memory.h"

#define DEFAULT_FLAGS VMMFlag::PRESENT | VMMFlag::READ_WRITE

VMM::VMM() {
    this->m_pml4 = (page_table_t*) memory::get_allocator()->alloc_page();
    memset(m_pml4, 0, 0x1000);
}

VMM::~VMM() {

}

void VMM::map_memory(void* phys, void* virt) {
    map_memory(phys, virt, DEFAULT_FLAGS);
}

void VMM::map_memory(void* phys, void* virt, uint64_t flags) {
    /* Finding the indices from the virtual address */
    uint64_t indices[4] = { 0, 0, 0, 0 }; // Holds indices in order: PDP, PD, PT, P

    uint64_t curr = (uint64_t) virt;
    for(int i = 0; i < 4; i++) {
        if(i == 0) curr >>= 12;
        else curr >>= 9;

        indices[3 - i] = curr & 0x1FF;
    }
    
    /* Traversing the tables, creating new ones if necessary */
    pde_t PDE = m_pml4->entries[indices[0]];

    page_table_t* last_table = m_pml4;
    page_table_t* table;
    for(int i = 0; i < 3; i++) {
        if (!PDE.get_flag(VMMFlag::PRESENT)) { // If the table is not present then we need to create a new one
            /* Allocate space for a table */
            table = (page_table_t*) memory::get_allocator()->alloc_page();
            memset((void*) table, 0, 0x1000);

            PDE.set_address((uint64_t) table >> 12); // Set the PDE's address to the address which we allocated
            PDE.set_flags(DEFAULT_FLAGS, true);

            // Finally, set the entry in the PML4 to this new PDP
            last_table->entries[indices[i]] = PDE;
        } else {
            table = (page_table_t*)((uint64_t) PDE.get_address() << 12);
        }

        PDE = table->entries[indices[i + 1]];

        last_table = table;
    }

    PDE.set_address((uint64_t) phys >> 12);
    PDE.set_flags(flags, true);
    table->entries[indices[3]] = PDE;
}

void* VMM::v2p(void* virtual_address) {
    uint64_t aligned_virtual_address = ((uint64_t) virtual_address / 0x1000) * 0x1000;

    /* Finding the indices from the virtual address */
    uint64_t indices[4] = { 0, 0, 0, 0 }; // Holds indices in order: PDP, PD, PT, P

    uint64_t curr = (uint64_t) virtual_address;
    for(int i = 0; i < 4; i++) {
        if(i == 0) curr >>= 12;
        else curr >>= 9;

        indices[3 - i] = curr & 0x1FF;
    }
    
    /* Traversing the tables, creating new ones if necessary */
    pde_t PDE = m_pml4->entries[indices[0]];

    page_table_t* last_table = m_pml4;
    page_table_t* table;
    for(int i = 0; i < 3; i++) {
        if (!PDE.get_flag(VMMFlag::PRESENT)) { // If the table is not present then the page hasn't been mapped
            return nullptr;
        } else {
            table = (page_table_t*)((uint64_t) PDE.get_address() << 12);
        }

        PDE = table->entries[indices[i + 1]];

        last_table = table;
    }

    uint64_t physical_address = PDE.get_address() << 12;
    return (void*) (physical_address + (((uint64_t) virtual_address) % 0x1000));
}

page_table_t* VMM::get_pml4() {
    return m_pml4;
}

void pde_t::set_flag(VMMFlag flag, bool enabled) {
    uint64_t bit_selector = (uint64_t) 1 << flag;
    this->value &= ~bit_selector;
    if(enabled) {
        this->value |= bit_selector;
    }
}

// Hi, Felix put this here :)
void pde_t::set_flags(uint64_t flags, bool enabled) {
    if(enabled) this->value |= flags;
    else this->value &= (~flags);
}

bool pde_t::get_flag(VMMFlag flag) {
    return value & (uint64_t) flag > 0 ? true : false;
}

uint64_t pde_t::get_address(){
    return (value & 0x000FFFFFFFFFF000) >> 12;
}

void pde_t::set_address(uint64_t address) {
    address &= 0x000000FFFFFFFFFF;
    this->value &= 0xFFF0000000000FFF;
    this->value |= (address << 12);
}
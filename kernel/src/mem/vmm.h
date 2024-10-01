#pragma once

#include <stdint.h>

enum VMMFlag {
    PRESENT = 0b1,                // 0
    READ_WRITE = 0b10,           // 1
    USER_SUPER = 0b100,          // 2
    WRITE_THROUGH = 0b1000,      // 3
    CACHE_DISABLED = 0b10000,    // 4
    ACCESSED = 0b100000,         // 5
    LARGER_PAGES = 0b10000000,   // 7
    CUSTOM_0 = 0b1000000000,     // 9
    CUSTOM_1 = 0b10000000000,    // 10
    CUSTOM_2 = 0b100000000000,   // 11
    NX = 63
};

struct pde_t {
    uint64_t value;
    
    void set_flag(VMMFlag flag, bool enabled);
    void set_flags(uint64_t flags, bool enabled);
    bool get_flag(VMMFlag flag);

    void set_address(uint64_t address);
    uint64_t get_address();
} __attribute__((packed));

struct page_table_t { 
    pde_t entries[512];
}__attribute__((aligned(0x1000))) __attribute__((packed));

class VMM {
    private:
        page_table_t* m_pml4;
    public:
        VMM();
        ~VMM();

        void map_memory(void* phys, void* virt);
        void map_memory(void* phys, void* virt, uint64_t flags);
        void* v2p(void* virtual_address);

        page_table_t* get_pml4();
};
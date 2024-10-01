#pragma once

#include <stdint.h>
#include <stdbool.h>

enum PTFlag {
    PRESENT = 0,
    READ_WRITE = 1,
    USER_SUPER = 2,
    WRITE_THROUGH = 3,
    CACHE_DISABLED = 4,
    ACCESSED = 5,
    LARGER_PAGES = 7,
    CUSTOM_0 = 9,
    CUSTOM_1 = 10,
    CUSTOM_2 = 11,
    NX = 63 // only if supported
};

typedef struct __attribute__((packed)) {
    uint64_t value;
} pde_t;

typedef struct __attribute__((aligned(0x1000))) __attribute__((packed)) { 
    pde_t entries[512];
} page_table_t;

void map_memory(page_table_t* pml4, void* phys, void* virt);

void set_flag(pde_t* pde, int flag, bool enabled);
bool get_flag(pde_t* pde, int flag);
uint64_t get_address(pde_t* pde);
void set_address(pde_t* pde, uint64_t address);
#pragma once

#include "stdint.h"

struct gdt_ptr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__ ((packed));

struct gdt_entry_t {
    uint16_t limit_lo;
    uint16_t base_lo;
    uint8_t base_mid;
    uint8_t access;
    uint8_t limit_hi_and_flags;
    uint8_t base_hi;
} __attribute__((packed));

struct gdt_t {
    gdt_entry_t entries[6];
} __attribute__((packed));

extern "C" void _load_gdt(gdt_ptr_t* gdtr);

class GDT {
    private:
        gdt_t* m_gdt;
    public:
        GDT();
        ~GDT();

        void install();
    private:
        void copy_low();
};

extern gdt_ptr_t* g_gdtr;
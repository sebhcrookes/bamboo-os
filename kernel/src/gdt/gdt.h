#pragma once

#include "stdint.h"

struct gdt_ptr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__ ((packed));

struct gdt_entry_t {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
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
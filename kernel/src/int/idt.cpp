#include "idt.h"

namespace idt {
    __attribute__((aligned(0x10)))
    idt_entry_t idt[256];

    idtr_t idtr;

    void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
        idt_entry_t* descriptor = &idt[vector];
    
        descriptor->isr_low = (uint64_t)isr & 0xFFFF;
        descriptor->kernel_cs = 0x08;
        descriptor->ist = 0;
        descriptor->attributes = flags;
        descriptor->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
        descriptor->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
        descriptor->reserved = 0;
    }

    void load_idtr() {
        idtr.base = (uint64_t) &idt[0];
        idtr.limit = (uint16_t) sizeof(idt_entry_t) * MAX_IDT_ENTRIES - 1;

        __asm__ volatile ("lidt %0" : : "m"(idtr));
    }
}
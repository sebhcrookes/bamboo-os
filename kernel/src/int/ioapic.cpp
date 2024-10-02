#include "ioapic.h"

#include <stdio.h>
#include "../mem/memory.h"

// Memory mapped registers for IO APIC register access
#define IOREGSEL                        0x00
#define IOWIN                           0x10

// IO APIC Registers
#define IOAPICID                        0x00
#define IOAPICVER                       0x01
#define IOAPICARB                       0x02
#define IOREDTBL                        0x10

namespace ioapic {

    uint64_t ioapic_addr;

    static void out(uint64_t base, uint8_t reg, uint32_t val) {
        io::mmio_write_32((void*)(base + IOREGSEL), reg);
        io::mmio_write_32((void*)(base + IOWIN), val);
    }

    static uint32_t in(uint64_t base, uint8_t reg) {
        io::mmio_write_32((void*)(base + IOREGSEL), reg);
        return io::mmio_read_32((void*)(base + IOWIN));
    }

    void init() {
        /* Getting a new page and mapping the old address onto it */
        uint64_t virt_addr = (uint64_t) memory::get_allocator()->alloc_page();
        memory::get_vmm()->map_memory((void*)((uint64_t)(ioapic_addr / 0x1000) * 0x1000), (void*) virt_addr);
        virt_addr += ioapic_addr % 0x1000;

        ioapic_addr = virt_addr;

        // Get number of entries supported by the IO APIC
        uint32_t x = in(ioapic_addr, IOAPICVER);
        uint32_t count = ((x >> 16) & 0xFF) + 1;

        // Disable all entries
        for (int i = 0; i < count; ++i) {
            set_entry(ioapic_addr, i, 1 << 16);
        }
    }

    void setup_redtbl() {
        set_entry(ioapic_addr, 0, 0);
    }

    void set_entry(uint64_t base, uint8_t index, uint64_t data) {
        out(base, IOREDTBL + index * 2, (uint32_t) data);
        out(base, IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
    }


    void set_addr(uint64_t address) {
        ioapic_addr = address;
    }

    uint64_t get_addr() {
        return ioapic_addr;
    }

    void eoi() {
        io::mmio_write_32((void*) 0xfee000b0, 0);
    }
}
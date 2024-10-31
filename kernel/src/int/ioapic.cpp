#include "ioapic.h"

#include <stdio.h>
#include "../mem/memory.h"

namespace ioapic {

    // Memory mapped registers for IO APIC register access
    #define IOWIN 0x10

    // IO APIC Registers
    #define IOAPICVER 0x01
    #define IOREDTBL 0x10

    uint64_t ioapic_addr;

    void init() {
        /* Getting a new page and mapping the old address onto it */
        uint64_t virt_addr = (uint64_t) memory::get_allocator()->alloc_page();
        memory::get_vmm()->map_memory((void*)((uint64_t)(ioapic_addr / 0x1000) * 0x1000), (void*) virt_addr);
        virt_addr += ioapic_addr % 0x1000;

        ioapic_addr = virt_addr;

        /* Get number of entries supported by the IO APIC */
        uint32_t ioapicver = in(ioapic_addr, IOAPICVER); // Bits 0-7 store I/O APIC version, bits 16-23 store the "Max Redirection Entry"

        // Maximum redirection entry = how many IRQs this I/O APIC can handle - 1
        uint32_t count = ((ioapicver >> 16) & 0xFF) + 1;

        /* Disable all entries. Some are then enabled later on in interrupts.cpp */
        for (int i = 0; i < count; i++) {
            set_entry(ioapic_addr, i, 1 << 16); // The 16th bit is the mask bit - if we set the mask bit then the IRQ is disabled
        }
    }

    void setup_redtbl() {
        set_entry(ioapic_addr, 0, 0);
    }

    void set_entry(uint64_t base, uint8_t index, uint64_t data) {
        out(base, IOREDTBL + index * 2, (uint32_t) data); // Lower 32 bits
        out(base, IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32)); // Higher 32 bits
    }

    void out(uint64_t base, uint8_t reg, uint32_t val) {
        io::mmio_write_32((void*)base, reg); // Select the register to write to
        io::mmio_write_32((void*)(base + IOWIN), val); // Write the value
    }

    uint32_t in(uint64_t base, uint8_t reg) {
        io::mmio_write_32((void*)base, reg); // Set the register to read from
        return io::mmio_read_32((void*)(base + IOWIN)); // Read the value
    }

    void set_addr(uint64_t address) {
        ioapic_addr = address;
    }

    uint64_t get_addr() {
        return ioapic_addr;
    }

    void eoi() {
        io::mmio_write_32((void*) 0xFEE000B0, 0);
    }
}
#include "local_apic.h"

#include <stdlib.h>
#include <stdio.h>

#include "../mem/memory.h"

namespace local_apic {

    /* LAPIC registers */
    #define LAPIC_TPR 0x0080  // Task Priority Register
    #define LAPIC_LDR 0x00D0  // Logical Destination Register
    #define LAPIC_DFR 0x00E0  // Destination Format Register
    #define LAPIC_SVR 0x00F0  // Spurious Interrupt Vector Register

    uint32_t local_apic_addr;

    void init() {
        map_memory((void*) local_apic_addr, (void*) local_apic_addr, VMMFlag::PRESENT | VMMFlag::READ_WRITE | VMMFlag::CACHE_DISABLED);

        // Enable all interrupts
        out(LAPIC_TPR, 0);

        out(LAPIC_DFR, 0xFFFFFFFF);
        out(LAPIC_LDR, 0x01000000);

        // Spurious interrupt vector reister
        out(LAPIC_SVR, 0x100 | 0xFF);
    }

    uint32_t in(uint32_t reg) {
        return io::mmio_read_32((void*)(local_apic_addr + reg));
    }

    void out(uint32_t reg, uint32_t data) {
        io::mmio_write_32((void*)(local_apic_addr + reg), data);
    }

    void set_local_apic_addr(uint32_t address) {
        local_apic_addr = address;
    }

    uint32_t get_local_apic_addr() {
        return local_apic_addr;
    }

    uint32_t get_id() {
        return in(0x0020) >> 24;
    }
}
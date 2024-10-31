#pragma once

#include <stdint.h>

namespace local_apic {
    void init();

    uint32_t in(uint32_t reg);
    void out(uint32_t reg, uint32_t data);

    void set_local_apic_addr(uint32_t address);
    uint32_t get_local_apic_addr();

    uint32_t get_id();
}
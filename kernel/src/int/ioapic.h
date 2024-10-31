#pragma once

#include <stdint.h>

namespace ioapic {
    void init();

    void setup_redtbl();

    void set_entry(uint64_t base, uint8_t index, uint64_t data);

    void out(uint64_t base, uint8_t reg, uint32_t val);
    uint32_t in(uint64_t base, uint8_t reg);

    void set_addr(uint64_t address);
    uint64_t get_addr();

    void eoi();
}
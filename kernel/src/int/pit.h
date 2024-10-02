#pragma once

#include <stdint.h>

namespace pit {

    void init();

    void sleep(uint64_t ms);

    void set_divisor(uint16_t divisor);

    double get_frequency();
    void set_frequency(uint64_t frequency);

    void tick();
    uint64_t get_uptime();
}
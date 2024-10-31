#pragma once

#include <stdint.h>

namespace math {
    void init();

    int rand();
    void srand(unsigned int seed);
    uint8_t rand1();
    uint64_t rand_u64();
    int64_t rand_s64p();

    double pow(double base, uint64_t exponent);
}
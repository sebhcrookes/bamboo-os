#include "math.h"

namespace math {
    uint64_t next;

    void init() {
        srand(100);
    }

    int rand() { // RAND_MAX assumed to be 65535
        next = next * 1103515245 + 12345;
        return (unsigned int) (next / 131072) % 65536;
    }
    
    void srand(unsigned int seed) {
        next = seed;
    }

    uint8_t rand1() {
        int num = math::rand();

        return num & 0b0000000000000001; 
    }

    uint64_t rand_u64() {
        uint64_t num = rand();
        num <<= 16;
        num |= rand();
        num <<= 16;
        num |= rand();
        num <<= 16;
        num |= rand();

        return num;
    }

    /* Random signed 64-bit positive integer */
    int64_t rand_s64p() {
        uint64_t num = rand();
        num <<= 15; // Leaving the MSb as 0
        num |= rand();
        num <<= 16;
        num |= rand();
        num <<= 16;
        num |= rand();

        return num;
    }

    // Only works with positive, non-fractional exponents
    double pow(double base, uint64_t exponent) {
        double value = 1;

        for(int i = 0; i < exponent; i++) {
            value *= base;
        }

        return value;
    }
}
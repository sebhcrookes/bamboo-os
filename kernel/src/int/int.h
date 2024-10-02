#pragma once

#include <stdint.h>

namespace interrupts {

    #define IRQ_TIMER 0
    #define INT_TIMER 0x20

    void init();

    void remap_interrupts();

    void enable();
    void disable(); 
}

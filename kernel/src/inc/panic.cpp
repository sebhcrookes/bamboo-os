#include "panic.h"

#include <stdio.h>

#include "../int/int.h"

namespace faults {
    void assert(bool condition, const char* reason) {
        if(!condition) {
            panic(reason);
        }
    }

    void panic(const char* reason) {
        interrupts::disable();

        io::printf("Kernel Panic: something went wrong which means that the system cannot continue running. The reason for the crash is:\n\n%s", reason);
        while(true) {
            asm("hlt");
        }
    }
}
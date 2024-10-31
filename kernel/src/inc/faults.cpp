#include "faults.h"

#include <stdio.h>

#include "../int/interrupts.h"

namespace faults {
    void assert(bool condition, const char* reason) {
        if(!condition) {
            panic(reason);
        }
    }

    void panic(const char* reason) {

        interrupts::disable();

        io::clear_screen(0x14232d);
       
        io::printf(" _  __                    _   ____             _      \n");
        io::printf("| |/ /___ _ __ _ __   ___| | |  _ \\ __ _ _ __ (_) ___ \n");
        io::printf("| ' // _ \\ '__| '_ \\ / _ \\ | | |_) / _` | '_ \\| |/ __|\n");
        io::printf("| . \\  __/ |  | | | |  __/ | |  __/ (_| | | | | | (__ \n");
        io::printf("|_|\\_\\___|_|  |_| |_|\\___|_| |_|   \\__,_|_| |_|_|\\___|\n");
        io::printf("______________________________________________________\n");

        io::printf("\n\nA fatal error occurred which BambooOS is unable to recover from. For this reason, your device needs to restart.\nPlease take note of the following reason for the crash.\n\nReason: %s", reason);
       
        while(true) {
            asm("hlt");
        }
    }
}
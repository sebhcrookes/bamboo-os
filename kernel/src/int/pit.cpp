#include "pit.h"

#include <stdio.h>

#define CMD_BINARY 0x00 // Use Binary counter values
#define CMD_MODE3 0x06 // Square Wave
#define CMD_RW_BOTH 0x30 // Least followed by Most Significant Byte
#define CMD_COUNTER0 0x00

namespace pit {
    uint64_t uptime = 0;

    uint16_t pit_divisor = 65535;

    void init() {
        set_divisor(1193);
    }

    void sleep(uint64_t us) {
        uint64_t end = us + uptime;
        while(end >= uptime) {
            asm("hlt");
        }
    }

    void set_divisor(uint16_t divisor) {
        io::io_write_8(0x43, CMD_BINARY | CMD_MODE3 | CMD_RW_BOTH | CMD_COUNTER0);
        io::io_write_8(0x40, (uint8_t)(divisor & 0x00FF));
        io::io_wait();
        io::io_write_8(0x40, (uint8_t)((divisor & 0xFF00) >> 8));
    }

    void tick() {
        uptime++;
    }

    uint64_t get_uptime() {
        return uptime;
    }
}
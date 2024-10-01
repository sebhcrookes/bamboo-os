#include "pit.h"

#include <stdio.h>

// BCD
#define CMD_BINARY                      0x00    // Use Binary counter values
#define CMD_BCD                         0x01    // Use Binary Coded Decimal counter values

// Mode
#define CMD_MODE0                       0x00    // Interrupt on Terminal Count
#define CMD_MODE1                       0x02    // Hardware Retriggerable One-Shot
#define CMD_MODE2                       0x04    // Rate Generator
#define CMD_MODE3                       0x06    // Square Wave
#define CMD_MODE4                       0x08    // Software Trigerred Strobe
#define CMD_MODE5                       0x0a    // Hardware Trigerred Strobe

// Read/Write
#define CMD_LATCH                       0x00
#define CMD_RW_LOW                      0x10    // Least Significant Byte
#define CMD_RW_HI                       0x20    // Most Significant Byte
#define CMD_RW_BOTH                     0x30    // Least followed by Most Significant Byte

// Counter Select
#define CMD_COUNTER0                    0x00
#define CMD_COUNTER1                    0x40
#define CMD_COUNTER2                    0x80
#define CMD_READBACK                    0xc0


namespace pit {
    uint64_t time_since_boot = 0;

    const uint64_t BaseFrequency = 1193182;

    uint16_t divisor = 65535;

    void init() {
        // set_frequency(1000);
        set_divisor(1193);
    }

    void sleep(uint64_t us) {
        uint64_t end = us + time_since_boot;
        while(end >= time_since_boot) {
            asm("hlt");
        }
    }

    void set_divisor(uint16_t Divisor) {
        if (Divisor < 100) Divisor = 100;
        divisor = Divisor;
        io::io_write_8(0x43, CMD_BINARY | CMD_MODE3 | CMD_RW_BOTH | CMD_COUNTER0);
        io::io_write_8(0x40, (uint8_t)(divisor & 0x00ff));
        io::io_wait();
        io::io_write_8(0x40, (uint8_t)((divisor & 0xff00) >> 8));
    }

    void tick() {
        time_since_boot++;
    }

    uint64_t get_uptime() {
        return time_since_boot;
    }
}
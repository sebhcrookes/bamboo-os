#include "pic.h"

#include <stdio.h>

namespace pic {

    #define PIC1_CMD                        0x0020
    #define PIC1_DATA                       0x0021
    #define PIC2_CMD                        0x00a0
    #define PIC2_DATA                       0x00a1

    #define ICW1_ICW4                       0x01        // ICW4 command word: 0 = not needed, 1 = needed
    #define ICW1_INIT                       0x10
    #define ICW4_8086                       0x01        // Microprocessor mode: 0=MCS-80/85, 1=8086/8086

    void disable() {
        // ICW1: start initialization, ICW4 needed
        io::io_write_8(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
        io::io_write_8(PIC2_CMD, ICW1_INIT | ICW1_ICW4);

        // ICW2: interrupt vector address
        io::io_write_8(PIC1_DATA, 0x20);
        io::io_write_8(PIC2_DATA, 0x20 + 8);

        // ICW3: master/slave wiring
        io::io_write_8(PIC1_DATA, 4);
        io::io_write_8(PIC2_DATA, 2);

        // ICW4: 8086 mode, not special fully nested, not buffered, normal EOI
        io::io_write_8(PIC1_DATA, ICW4_8086);
        io::io_write_8(PIC2_DATA, ICW4_8086);

        // OCW1: Disable all IRQs
        io::io_write_8(PIC1_DATA, 0xff);
        io::io_write_8(PIC2_DATA, 0xff);
    }
}
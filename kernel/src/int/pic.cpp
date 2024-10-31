#include "pic.h"

#include <stdio.h>

namespace pic {

    #define PIC1_CMD 0x0020
    #define PIC1_DATA 0x0021
    #define PIC2_CMD 0x00a0
    #define PIC2_DATA 0x00a1

    #define ICW1_ICW4 0x01
    #define ICW1_INIT 0x10
    #define ICW4_8086 0x01

    void disable() {
        // Start the initialisation sequence
        io::io_write_8(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
        io::io_wait();
        io::io_write_8(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
        io::io_wait();

        // PIC vector offsets
        io::io_write_8(PIC1_DATA, 0x20);
        io::io_wait();
        io::io_write_8(PIC2_DATA, 0x28);
        io::io_wait();

        // Tell the master PIC there is a slave PIC at IRQ2
        io::io_write_8(PIC1_DATA, 4);
        io::io_wait();

        // Tell the slave PIC its cascade identity
        io::io_write_8(PIC2_DATA, 2);
        io::io_wait();

        // Tell the PICs to use 8086 mode
        io::io_write_8(PIC1_DATA, ICW4_8086);
        io::io_wait();
        io::io_write_8(PIC2_DATA, ICW4_8086);
        io::io_wait();

        // Mask all IRQs to disable the PIC chip
        io::io_write_8(PIC1_DATA, 0xff);
        io::io_wait();
        io::io_write_8(PIC2_DATA, 0xff);
        io::io_wait();
    }
}
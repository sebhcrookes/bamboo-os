#pragma once

#include "../renderer.h"

#define COL_CONSOLE_BACKGROUND 0x1C1613
#define COL_KERNEL_PRINT 0x48A5C2
#define COL_PRINT 0xFFE8F2
#define COL_SPECIAL_PRINT 0x97C8EB
#define COL_DRIVER_PRINT 0x64E9EE
#define COL_ERROR_PRINT 0xFF0000

namespace io {
    void init(Renderer* renderer);

    void print_drv_name(const char* name);

    void printf(const char* format, ...);

    void clear_current_line();

    void clear_screen(int colour);

    void set_print_colour(int colour);

    /* CPU IO functions */

    void io_wait();

    void io_write_8(uint32_t port, uint8_t data);
    void io_write_32(uint32_t port, uint32_t data);

    uint32_t io_read_32(uint32_t port);

    void mmio_write_32(void *p, uint32_t data);
    uint32_t mmio_read_32(void *p);
}
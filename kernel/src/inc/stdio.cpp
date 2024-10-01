#include "stdio.h"

#include <stdarg.h>

namespace io {
    Renderer* r;

    void init(Renderer* renderer) {
        r = renderer;
    }

    /* Prints the driver name before a message is printed from the driver */
    void print_drv_name(const char* name) {
        r->putc('[', COL_DRIVER_PRINT);
        r->puts(name, COL_DRIVER_PRINT);
        r->putc(']', COL_DRIVER_PRINT);
        r->puts(" - ", COL_PRINT);
    }

    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);

        for(int i = 0; format[i] != 0; i++) {
            if(format[i] == '%') {
                switch(format[i + 1]) {
                    case 'c': {
                        char val = (char) va_arg(args, int); // Char is promoted to int when passed through '...'
                        r->putc(val, COL_PRINT);
                    } break;
                    case 's': {
                        const char* val = va_arg(args, const char*);
                        r->puts(val, COL_PRINT);
                    } break;
                    case 'u': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 10, false, false, COL_PRINT);
                    } break;
                    case 'd': {
                        int64_t val = va_arg(args, int64_t);
                        r->putd(val, 10, true, false, COL_PRINT);
                    } break;
                    case 'X': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 16, false, true, COL_PRINT);
                    } break;
                    case 'x': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 16, false, false, COL_PRINT);
                    } break;
                }
                i++;
            } else {
                r->putc(format[i], COL_PRINT);
            }
        }

        va_end(args);
    }

    void clear_current_line() {
        r->clear_current_line();
    }

    /* ========= CPU IO functions ========= */

    void io_wait() {
        asm volatile ("outb %%al, $0x80" : : "a"(0));
    }

    void io_write_8(uint32_t port, uint8_t data) {
        __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
    }

    void mmio_write_32(void *p, uint32_t data) {
        *(volatile uint32_t*)(p) = data;
    }

    uint32_t mmio_read_32(void *p) {
        return *(volatile uint32_t*)(p);
    }
}
#include "stdio.h"

#include <stdarg.h>

namespace io {
    Renderer* r;

    int print_colour;

    void init(Renderer* renderer) {
        r = renderer;

        print_colour = COL_PRINT;
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
                        r->putc(val, print_colour);
                    } break;
                    case 's': {
                        const char* val = va_arg(args, const char*);
                        r->puts(val, print_colour);
                    } break;
                    case 'u': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 10, false, false, print_colour);
                    } break;
                    case 'd': {
                        int64_t val = va_arg(args, int64_t);
                        r->putd(val, 10, true, false, print_colour);
                    } break;
                    case 'X': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 16, false, true, print_colour);
                    } break;
                    case 'x': {
                        uint64_t val = va_arg(args, uint64_t);
                        r->putd(val, 16, false, false, print_colour);
                    } break;
                }
                i++;
            } else {
                r->putc(format[i], print_colour);
            }
        }

        va_end(args);
    }

    void clear_current_line() {
        r->clear_current_line();
    }

    void clear_screen(int colour) {
        r->clear(colour);
    }

    void set_print_colour(int colour) {
        print_colour = colour;
    }

    /* ========= CPU IO functions ========= */

    void io_wait() {
        asm volatile ("outb %%al, $0x80" : : "a"(0));
    }

    void io_write_8(uint32_t port, uint8_t data) {
        __asm__ volatile("outb %b0, %w1" : : "a" (data), "Nd" (port));
    }

    void io_write_32(uint32_t port, uint32_t data) {
        asm volatile("outl %%eax, %%dx": : "a" (data), "Nd" (port));
    }

    uint32_t io_read_32(uint32_t port) {
        uint32_t ret;
        asm volatile("inl %%dx, %%eax" : "=a" (ret) : "Nd" (port));
        return ret;
    }

    void mmio_write_32(void *p, uint32_t data) {
        *(volatile uint32_t*)(p) = data;
    }

    uint32_t mmio_read_32(void *p) {
        return *(volatile uint32_t*)(p);
    }
}
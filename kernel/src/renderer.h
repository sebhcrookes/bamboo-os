#pragma once

#include "kernel.h"

#include "boot_structs.h"

#include <stdbool.h>
#include <stdint.h>

class Renderer {
    private:
        bamboo_boot_info_t* m_boot_info;
        bamboo_gop_t* m_framebuffer;

        int m_clear_colour;
    public:
        Renderer();
        Renderer(bamboo_boot_info_t* boot_info);
        ~Renderer();

        uint64_t twos_comp_to_dec(int64_t val);

        void puts(const char* str, int colour);
        void putd(uint64_t val, uint8_t base, bool sign, bool capitalise, int colour);
        void putc(char c, int colour);
        void set_pixel(int x, int y, int colour);

        void clear_current_line();
        void clear(int colour);
};

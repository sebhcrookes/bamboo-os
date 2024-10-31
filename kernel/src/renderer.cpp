#include "renderer.h"

#include <string.h>

bool graphical = false;

int pos_x = 0;
int pos_y = 0;

bool on_half_two = false;
bool is_splitscreen = false;

Renderer::Renderer() {
    
}

Renderer::Renderer(bamboo_boot_info_t* boot_info) {
    m_boot_info = boot_info;
    m_framebuffer = boot_info->framebuffer;
    m_clear_colour = 0;
}

Renderer::~Renderer() {

}

void Renderer::puts(const char* str, int colour) {
    for(const char* c = str; *c != 0; c++) {
        this->putc(*c, colour);
    }
}

uint64_t Renderer::twos_comp_to_dec(int64_t val) {
    if(val >= 0) // It is positive, so we can just return the value
        return (uint64_t) val;

    // It is negative, so convert from twos-complement to a normal binary uint
    return ~((uint64_t) val) + 1;
}


char base_alpha_caps[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char base_alpha_lower[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

void Renderer::putd(uint64_t val, uint8_t base, bool sign, bool capitalise, int colour) {
    char buffer[100];
    uint8_t ptr = 0;
    uint64_t c = val;

    if(base > 16) return; // Highest supported

    bool negative = false;

    if(sign) {
        c = twos_comp_to_dec(val);
        if((int64_t) val < 0) {
            negative = true;
        }
    }

    do {
        uint8_t rem = c % base;
        c = c / base;

        // Getting the character representation after the division
        char chr = 0;
        if(capitalise) chr = base_alpha_caps[rem];
        else chr = base_alpha_lower[rem];

        buffer[ptr] = chr;
        ptr++;
    } while(c != 0);

    if(negative) putc('-', colour);

    for(int i = ptr - 1; i >= 0; i--) {
        putc(buffer[i], colour);
    }
}

void Renderer::set_pixel(int x, int y, int colour) {
    *((uint32_t*)((uint64_t) m_framebuffer->base_address + 4 * m_framebuffer->pixels_per_scanline * y + 4 * x)) = colour;
}

void Renderer::putc(char c, int colour) {
    if(graphical) return; // You cannot use putc in graphical mode without specifying co-ordinates
    psf2_font_t* font = m_boot_info->font;
    // if(c == '\b') {
    //     pos_x -= font->psf2_header->width;
    //     for(int y = 0; y < font->psf2_header->height; y++) {
    //         for(int x = 0; x < font->psf2_header->width; x++) {
    //             set_pixel(pos_x + x, pos_y + y, colours::console_background);
    //         }
    //     }
    //     return;
    // }

    if(c == '\n' || ((pos_x == m_framebuffer->width / 2 - font->psf2_header->width) && is_splitscreen) || (on_half_two && pos_x >= m_framebuffer->width - font->psf2_header->width && is_splitscreen) || (pos_x >= m_framebuffer->width - font->psf2_header->width && !is_splitscreen)) {
        pos_y += font->psf2_header->height;

        if(pos_y >= m_framebuffer->height - font->psf2_header->height) {
            if(is_splitscreen) {
                on_half_two = !on_half_two;

                if(on_half_two) {
                    pos_x = m_framebuffer->width / 2;
                    pos_y = 0;
                } else {
                    memset(m_framebuffer->base_address, 0, m_framebuffer->pixels_per_scanline * 4 * m_framebuffer->height);
                    pos_x = 0;
                    pos_y = 0;
                }
            } else {
                memset(m_framebuffer->base_address, 0, m_framebuffer->pixels_per_scanline * 4 * m_framebuffer->height);
                pos_x = 0;
                pos_y = 0;
            }
        }
        if(on_half_two) {
            pos_x = m_framebuffer->width / 2;
        } else {
            pos_x = 0;
        }
        if(c == '\n') return;
    }

    uint8_t *glyph = &font->glyphs[c * font->psf2_header->bytesperglyph];
    uint32_t stride = font->psf2_header->bytesperglyph / font->psf2_header->height;
    uint32_t y0;
    uint32_t x0;

    if (glyph == 0)
        return;
    
    for (y0 = 0; y0 < font->psf2_header->height; ++y0) {
        for (x0 = 0; x0 < font->psf2_header->width; ++x0) {
            uint8_t bits = glyph[y0 * stride + x0 / 8];
            uint8_t bit = bits >> (7 - x0 % 8) & 1;
            if (bit) set_pixel(x0 + pos_x, y0 + pos_y, colour);
        }
    }

    pos_x += font->psf2_header->width;
}

void Renderer::clear_current_line() {
    memset32((void*)((uint64_t) m_framebuffer->base_address + m_framebuffer->pixels_per_scanline * 4 * pos_y), m_clear_colour, m_framebuffer->pixels_per_scanline * m_boot_info->font->psf2_header->height);
    pos_x = 0;
}

void Renderer::clear(int colour) {
    m_clear_colour = colour;
    memset32(m_framebuffer->base_address, colour, m_framebuffer->pixels_per_scanline * m_framebuffer->height);

    pos_x = 0;
    pos_y = 0;
}
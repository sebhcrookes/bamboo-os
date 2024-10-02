#pragma once

#include <stdint.h>

struct bitmap_header_t {
    uint64_t number_of_pages; // Not including this one
} __attribute__((packed));

class Bitmap {
    private:
        uint64_t m_num_pages;

        uint8_t m_start_reserved;

        void set_page(uint64_t upn, uint8_t value);
    public:
        Bitmap();
        Bitmap(uint8_t reserve);
        ~Bitmap();

        void lock_page(uint64_t upn);
        void free_page(uint64_t upn);

        uint64_t get_first_free();

        uint64_t get_num_pages();
};
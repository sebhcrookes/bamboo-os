#include "bitmap.h"

#include "memory.h"

#include <stdio.h>

#define BITS_PER_PAGE 32768 // 4096 * 8

/* The bitmap stores whether pages in the system are currently in use or free.

   The bitmap reserves some first pages as the operating system needs these for other tasks.

   It uses the next 'n' free pages as actual bitmap space, where n is the number of pages
   given in member variable "m_num_pages". It does not use the allocator's "alloc_page" function
   to get these pages as the allocator has not been set up fully yet. Instead, the bitmap should
   use the "upn_to_address" function to get the pages which it uses.

   The bitmap only worries about usable pages, and not about pages which cannot be used.
*/

Bitmap::Bitmap() {}

Bitmap::Bitmap(uint8_t reserve) {
    /* Calculate the number of pages needed for the bitmap */
    uint64_t mem_size = memory::get_usable_memory_size(); // Size in pages, so this is also the number of bits we need
    uint64_t bytes_needed = mem_size / 8; // Convert number of bits to number of bytes
    uint64_t pages_needed = bytes_needed / 4096; // Convert number of bytes to pages

    m_num_pages = pages_needed + 1; // Add an extra page (because number will have been rounded down)

    m_start_reserved = reserve;

    /* Lock the pages in use by the bitmap */
    for(int i = 0; i < reserve; i++) {
        lock_page(i); // The pages reserved for other OS tasks
    }

    for(int i = 0; i < m_num_pages; i++) {
        lock_page(reserve + i);
    }
}

Bitmap::~Bitmap() {

}

/* Sets the status of the usable page indexed by page number to be locked / in use */
void Bitmap::lock_page(uint64_t upn) {
    set_page(upn, 1);
}

/* Sets the status of the usable page indexed by page number to be free */
void Bitmap::free_page(uint64_t upn) {
   set_page(upn, 0);
}

void Bitmap::set_page(uint64_t upn, uint8_t value) {
    if(value != 0 && value != 1) return;

    uint64_t bmp_page = (upn / BITS_PER_PAGE) + m_start_reserved; // Add value to account for OS reserved pages
    uint64_t bit_offset = upn % BITS_PER_PAGE; // The offset (in bits) into the page
    uint64_t byte_offset = bit_offset / 8; // The byte we need to access
    uint64_t bit_offset_in_byte = bit_offset % 8; // The bit in that byte we need to access

    if(bmp_page - 1 > m_num_pages) return; // Is the page in the bitmap range

    void* page = memory::upn_to_address(bmp_page);
    
    if(value == 0) ((uint8_t*)(page))[byte_offset] &= ~(1UL << bit_offset_in_byte);
    else ((uint8_t*)(page))[byte_offset] |= 1UL << bit_offset_in_byte;
}

uint64_t Bitmap::get_first_free() {
    /* Here, we go through every bit of the bitmap to find one which is free */
    uint64_t mem_size = memory::get_usable_memory_size();
    uint64_t upn = 0;
    for(int page = 0; page < m_num_pages; page++) {
        for(int byte = 0; byte < 0x1000; byte++) {
            for(int bit = 0; bit < 8; bit++) {
                void* p = memory::upn_to_address(page + m_start_reserved);
                bool set = (((uint8_t*)(p))[byte] >> bit) & 1U;
                if(!set) return upn;

                upn++;
                if(upn > mem_size) return 0;
            }
        }
    }

    return 0;
}

uint64_t Bitmap::get_num_pages() {
    return m_num_pages;
}
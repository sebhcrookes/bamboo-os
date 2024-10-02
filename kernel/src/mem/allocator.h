#pragma once

#include "../boot_structs.h"
#include "bitmap.h"

class PageAllocator {
    private:
        bamboo_mmap_t* m_mmap;

        Bitmap m_bitmap;
    public:
        PageAllocator();
        PageAllocator(bamboo_mmap_t* mmap);
        ~PageAllocator();

        void* alloc_page();
        void free_page(void* address);

        void lock_page(void* address);
        void lock_pages(void* address, uint64_t n);
};
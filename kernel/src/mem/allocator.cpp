#include "allocator.h"

#include <stdio.h>

#include "memory.h"

PageAllocator::PageAllocator() {

}

PageAllocator::PageAllocator(bamboo_mmap_t* mmap) {
    this->m_mmap = mmap;
    this->m_bitmap = Bitmap(3);

    io::print_drv_name("PMM");
    io::printf("Initialised allocator, with %u bitmap pages\n", m_bitmap.get_num_pages());
}

PageAllocator::~PageAllocator() {

}

void* PageAllocator::alloc_page() {
    uint64_t page_number = m_bitmap.get_first_free();
    m_bitmap.lock_page(page_number);

    return memory::upn_to_address(page_number); 
}

void PageAllocator::free_page(void* address) {
    uint64_t page_number = memory::address_to_upn(address);
    m_bitmap.free_page(page_number);
}

void PageAllocator::lock_page(void* address) {
    uint64_t page_number = memory::address_to_upn(address);
    if(page_number == 0) return;

    m_bitmap.lock_page(page_number);
}

void PageAllocator::lock_pages(void* address, uint64_t n) {
    for(uint64_t i = 0; i < n * 0x1000; i += 0x1000) {
        lock_page((void*)((uint64_t) address + i));
    }
}

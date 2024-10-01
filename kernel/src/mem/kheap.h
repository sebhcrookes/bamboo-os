#pragma once

#include <stdint.h>

struct heap_segment_hdr_t {
    void* prev; // Points to the segment header of the previous segment
    void* next; // Points to the segment header of the next segment
    uint32_t size; // Holds actual size of segment, without header
    bool free;
}__attribute__((packed));

class KernelHeap {
    private:
        void* m_base_address;

        uint64_t m_size;
    public:
        KernelHeap();
        KernelHeap(void* base_address);
        ~KernelHeap();

        void* malloc(size_t size);
        void free(void* ptr);
        void* realloc(void* ptr, size_t size);
    
    private:
        void split_segment(heap_segment_hdr_t* curr, uint64_t size);
        void merge_forward(heap_segment_hdr_t* curr);
        void extend_heap(heap_segment_hdr_t* last, uint64_t num_pages);
};
#include "stdlib.h"

#include "../mem/memory.h"

/* Heap functions */

void* malloc(size_t size) {
    return memory::get_kheap()->malloc(size);
}

void free(void* ptr) {
    memory::get_kheap()->free(ptr);
}

void* realloc(void* ptr, size_t size) {
    return memory::get_kheap()->realloc(ptr, size);
}

/* VMM functions */

void map_memory(void* phys, void* virt) {
    memory::get_vmm()->map_memory(phys, virt);
}

void map_memory(void* phys, void* virt, uint64_t flags) {
    memory::get_vmm()->map_memory(phys, virt, flags);
}
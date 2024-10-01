#pragma once

#include <stdint.h>

#include "../mem/vmm.h"

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);

void map_memory(void* phys, void* virt);
void map_memory(void* phys, void* virt, uint64_t flags);

/* Implementation of the new and delete keywords in BambooOS */

inline void* operator new (size_t, void* ptr)      { return ptr; }
inline void* operator new[] (size_t size)    { return malloc(size); }
inline void* operator new (size_t size)          { return malloc(size); }

inline void operator delete (void* ptr)            { free(ptr); }
inline void operator delete (void* ptr, size_t)    { free(ptr); }
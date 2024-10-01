#pragma once

#include <stdint.h>

void* memset(void* dest, uint8_t val, uint64_t n);
void* memset32(void* dest, uint32_t val, uint64_t n);
void* memset128(void* dest, __uint128_t val, uint64_t n);
void memcpy(void* dest, void* src, uint64_t n);
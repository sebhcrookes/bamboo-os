#include "string.h"

void* memset(void* dest, uint8_t val, uint64_t n) {
    uint8_t* p = (uint8_t*) dest;
    while(n--) {
        *p++ = val;
    }
    return dest;
}

void* memset32(void* dest, uint32_t val, uint64_t n) {
    uint32_t* p = (uint32_t*) dest;
    while(n--) {
        *p++ = val;
    }
    return dest;
}

void* memset128(void* dest, __uint128_t val, uint64_t n) {
    __uint128_t* p = (__uint128_t*) dest;
    while(n--) {
        *p++ = val;
    }
    return dest;
}

void memcpy(void* dest, void* src, uint64_t n) {
    if(!n) return;

    while(n >= 16) {
        *(__uint128_t*) dest = *(__uint128_t*) src;
        dest = (uint8_t*) dest + 16;
        src = (uint8_t*) src + 16;
        n -= 16;
    }

    while(n >= 8) {
        *(uint64_t*) dest = *(uint64_t*) src;
        dest = (uint8_t*) dest + 8;
        src = (uint8_t*) src + 8;
        n -= 8;
    }

    while(n >= 1) {
        *(uint8_t*) dest = *(uint8_t*) src;
        dest = (uint8_t*) dest + 1;
        src = (uint8_t*) src + 1;
        n -= 1;
    }
}
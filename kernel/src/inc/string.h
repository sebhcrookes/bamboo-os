#pragma once

#include <stdint.h>

void* memset(void* dest, uint8_t val, uint64_t n);
void* memset32(void* dest, uint32_t val, uint64_t n);
void* memset128(void* dest, __uint128_t val, uint64_t n);
void memcpy(void* dest, void* src, uint64_t n);

bool strequ(char* str1, char* str2);
bool strequ(const char* str1, const char* str2);
bool strequ(char* str1, const char* str2);

size_t strlen(char *s);
size_t strlen(const char *s);

char* strtok(char* str, char delim);

char tolower(char chr);

char* strdup(char* src);

int atoi(char* str);
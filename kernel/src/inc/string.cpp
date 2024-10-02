#include "string.h"

#include "stdlib.h"

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

/* strequ */

bool strequ(char* str1, char* str2) {
    for(int i = 0; ; i++) {
        if (str1[i] != str2[i]) {
            return false;
        }

        if (str1[i] == '\0' && str2[i] == '\0') {
            return true;
        }
    }
}

bool strequ(const char* str1, const char* str2) {
    return strequ((char*) str1, (char*) str2);
}

bool strequ(char* str1, const char* str2) {
    return strequ((char*) str1, (char*) str2);
}

/* strlen */

size_t strlen(char* s) {
    size_t count = 0;
    while(*s != '\0') {
        count++;
        s++;
    }

    return count;
}

size_t strlen(const char* s) {
    return strlen((char*) s);
}

/* strtok */

char* input = nullptr;
char* strtok(char* s, char d) { 
    // Initialize the input string
    if (s != nullptr)
        input = s;
 
    // Case for final token
    if (input == 0)
        return nullptr;

    // Stores the extracted string
    char* result = (char*) malloc(strlen(input) + 1);

    int i = 0;
    for (; input[i] != '\0'; i++) {
 
        // If delimiter is not reached, keep going
        if (input[i] != d)
            result[i] = input[i];
        else { // Store the finished string + return
            result[i] = '\0';
            input = input + i + 1;
            return result;
        }
    }
 
    // When loop ends, add 0 character and set input to nullptr
    result[i] = '\0';
    input = nullptr;
 
    return result;
}

/* tolower */

char tolower(char chr) {
    if(chr >= 65 && chr <= 90) {
        return chr + 32;
    }
    return chr;
}

/* strdup */

char* strdup(char *src) {
    char *str;
    char *p;

    // Calculating string length
    int len = 0;
    while (src[len]) {
        len++;
    }

    // Allocating for new string
    str = (char*) malloc(len + 1);
    p = str;
    
    // Copying over all bytes
    while (*src) {
        *p++ = *src++;
    }

    *p = '\0';

    return str;
}

/* atoi */

int atoi(char* str) {
    int k = 0;
    while (*str) {
        k = (k << 3) + (k << 1) + (*str) - '0';
        str++;
    }
    return k;
}
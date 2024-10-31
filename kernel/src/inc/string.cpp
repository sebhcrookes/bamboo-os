#include "string.h"

#include "stdlib.h"
#include "math.h"

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

// This function WILL NOT WORK across multiple threads
char* input = nullptr;
char* next_token = nullptr;
char* strtok(char* str, char delim) { 
    /* The first time the function is called, str will contain the string being tokenised.
       Each time after that, str will be null. Therefore, we store the string in the input variable.
       next_token always points to the start of the next token. We then add in a null-termination
       character later on, duplicate the string and then return it. It is up to the caller to free
       the memory allocated by this function. */

    if(str != nullptr) {
        input = str;
        next_token = input;
    } else {
        if(next_token == nullptr) return nullptr;
    }

    // Finding next delimiter in the string
    int delim_pos = -1;
    for(int i = 0; i < strlen(next_token); i++) {
        if(next_token[i] == delim) {
            delim_pos = i;
            break;
        }
    }

    // Inserting a null-terminating character to split the string (or just returning if it is the last token)
    if(delim_pos == -1) { // The last token
        char* return_val = strdup(next_token);
        input = nullptr;
        next_token = nullptr;

        return return_val;
    } else {
        next_token[delim_pos] = '\0';
        char* return_val = strdup(next_token);
        next_token[delim_pos] = delim;
        
        next_token += delim_pos + 1;

        return return_val;
    }

    return nullptr;
}

/* tolower */

char tolower(char chr) {
    if(chr >= 'A' && chr <= 'Z') {
        return chr + ('a' - 'A');
    }

    return chr;
}

/* strdup */

char* strdup(char* src) {
    int len = strlen(src);

    // Allocate memory for the new string
    char* new_str = (char*) malloc(len + 1);

    // Copy over the characters to the new string
    for(int i = 0; i < len; i++) {
        new_str[i] = src[i];
    }

    new_str[len] = '\0'; // Null-terminate the string

    return new_str;
}

/* atoi */

int atoi(char* str) {
    int value = 0;
    int len = strlen(str);

    // Going from the end of the string to the start, adding on powers of 10
    int pow_10 = 0;
    for(int i = len - 1; i >= 0; i--) {
        if(i == 0 && str[i] == '-') { // Allowing for negative numbers
            value = -value;
            break;
        }

        value += (str[i] - '0') * math::pow(10, pow_10);
        pow_10++;
    }

    return value;
}
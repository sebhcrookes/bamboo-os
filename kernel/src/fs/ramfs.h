#pragma once

#include <stdint.h>

#include "vfs.h"

class RAMFS {
    private:
        /* data */
    public:
        RAMFS();

        file_t* fopen(const char* filename, const char* mode);

        uint8_t fwrite(file_t* file, char* buffer, uint64_t size);
        
};
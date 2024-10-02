#include "ramfs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

RAMFS::RAMFS() {

}


file_t* RAMFS::fopen(const char* filename, const char* mode) {
    file_t* f = (file_t*) malloc(sizeof(file_t));
    return f;
}

uint8_t RAMFS::fwrite(file_t* file, char* buffer, uint64_t size) {
    if(strequ(file->name, "stdout")) {
        for(uint64_t i = 0; i < size; i++) {
            io::printf("%c", buffer[i]);
        }
    }

    return RSUCCESS;
}
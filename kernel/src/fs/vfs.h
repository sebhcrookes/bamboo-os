#pragma once

#include <stdint.h>

#define FAT32_ID 0
#define RAMFS_ID 1

class Filesystem {
    private:
        uint32_t drive_label;
        uint16_t type;
        void* filesystem;
    public:
        Filesystem(uint32_t drive_label, uint16_t fstype, void* filesystem_class);
        ~Filesystem();

        char get_drive_label();
        uint16_t get_fstype();
        void* get_filesystem();
};

struct file_t {
    uint64_t file_desc;

    char* name;

    char* contents;
    uint64_t size;
    
    // --- Modes ---
    // read:    0b00000001
    // write:   0b00000010
    // append:  0b00000100
    
    uint8_t mode_flags;

    void* dirent;

    uint32_t drive_label;
};

struct fstat_t {
    uint64_t size;
};

namespace vfs {
    void init();

    uint64_t fopen(uint64_t* fd, const char* path, const char* mode);
    uint64_t fread(uint64_t fd, char* buffer);
    uint64_t fwrite(uint64_t fd, char* buffer, uint64_t size);
    uint64_t fclose(uint64_t fd);

    uint64_t fstat(uint64_t fd, fstat_t* statbuf);
    
    void fread(file_t* file, char* buffer);
    void fclose(file_t* file);

    uint32_t new_label();
    void add_fs(Filesystem* fs);
}
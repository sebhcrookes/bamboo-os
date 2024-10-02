#include "vfs.h"

#include <memory.h>
#include <string.h>
#include <vector.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <errno.h>

#include "storage.h"

#include "fat32.h"
#include "ramfs.h"

namespace vfs {

    Vector* filesystems;

    Vector* files; // Stores all currently open files + file descriptors

    void init() {
        filesystems = new Vector();
        files = new Vector();

        // Setting up the DEVFS with label 1
        RAMFS* ramfs = new RAMFS();
        Filesystem* devfs = new Filesystem(1, RAMFS_ID, (void*) ramfs);
        add_fs(devfs);

        Vector* stor_devs = storage::get_drivers();

        for(int i = 0; i < stor_devs->size(); i++) {
            storage::stor_drv_t* drv = (storage::stor_drv_t*) stor_devs->get(i);
            MSDOSPartitionTable* part_table = new MSDOSPartitionTable(drv->uuid);

            bool success = part_table->read_partitions();

            if(!success) {
                delete part_table;
                continue;
            }

            for(int i = 0; i < 4; i++) {
                PartitionTableEntry entry = part_table->get_mbr()->primaryPartition[i];
                if(entry.partition_id == 0) continue;

                FAT32* fat32 = new FAT32(part_table, i);
                Filesystem* fs = new Filesystem(new_label(), FAT32_ID, (void*) fat32);
                add_fs(fs);
            }
        }

        io::print_drv_name("VFS");
        io::printf("%u filesystem drivers initialised, with RAMFS test: ", filesystems->size());

        // Test of RAMFS' stdout
        uint64_t fd;
        vfs::fopen(&fd, "1:stdout", "w");
        vfs::fwrite(fd, "printing through RAMFS' stdout\n", 31);
        vfs::fclose(fd);
    }

    file_t* get_file(uint64_t fd) {
        file_t* file = nullptr;

        for(int i = 0; i < files->size(); i++) {
            file_t* current = (file_t*) files->get(i);
            
            if(current->file_desc == fd) {
                file = current;
                break;
            }
        }
        
        return file;
    }

    uint64_t fopen(uint64_t* fd, const char* path, const char* mode) {
        int pathlen = strlen(path);

        char* p = (char*) path;

        /* Getting the drive label from the start */

        // Paths consist of label:path, where label specifies the drive being accessed

        uint8_t label_len = 0;
        while(*p != ':' && *p != 0) { // Skipping characters until we reach the ':'
            if(*p < '0' || *p > '9') {
                return RINVARG;
            }
            label_len += 1;
            p++;
        }

        p -= label_len;
        p[label_len] = 0;

        uint32_t drive_label = atoi(p);
        
        p[label_len] = ':';

        uint8_t skip = label_len + 1;

        /* Now, call the correct function with the path found */

        if(pathlen < label_len + 1) // Invalid path (too short)
            return RINVARG;
        
        for(int i = 0; i < filesystems->size(); i++) {
            Filesystem* fs = (Filesystem*) filesystems->get(i);
            if(fs->get_drive_label() == drive_label) { // Find the correct filesystem
                uint16_t type = fs->get_fstype();
                
                file_t* retv;

                switch(type) {
                    case FAT32_ID: {
                        FAT32* fat32 = (FAT32*) fs->get_filesystem();
                        retv = fat32->fopen(path + skip, mode);
                    } break;
                    
                    case RAMFS_ID: {
                        RAMFS* ramfs = (RAMFS*) fs->get_filesystem();
                        retv = ramfs->fopen(path + skip, mode);
                    } break;

                    default: retv = nullptr;
                }

                if(retv != nullptr) {
                    retv->drive_label = drive_label;
                    
                    /* Generate + assign a file descriptor */
                    bool valid = true;
                    do {
                        // Get a new 64-bit random number
                        uint64_t fd = math::rand_u64();

                        // Assume fd is not already in use, then check if it is
                        valid = true;
                        for(uint64_t i = 0; i < files->size(); i++) {
                            file_t* f = (file_t*) files->get(i);
                            if(f->file_desc == fd) {
                                valid = false;
                                break;
                            }
                        }

                        // If fd is not in use, assign it to retv
                        if(valid) retv->file_desc = fd;
                    } while(!valid);
                    retv->name = strdup((char*) path + skip);
                    files->add((void*) retv);
                }

                *(fd) = retv->file_desc;
                return RSUCCESS;
            }
        }

        return RNOENT;
    }
    
    uint64_t fread(uint64_t fd, char* buffer) {
        file_t* file = get_file(fd);

        if(file == nullptr) return RINVFD; // No file present for given file descriptor

        char drive_label = file->drive_label;

        for(int i = 0; i < filesystems->size(); i++) {
            Filesystem* fs = (Filesystem*) filesystems->get(i);
            if(fs->get_drive_label() == drive_label) {
                uint16_t type = fs->get_fstype();
                
                if(type == FAT32_ID) {
                    FAT32* fat32 = (FAT32*) fs->get_filesystem();
                    fat32->fread(file, buffer);
                    return RSUCCESS;
                } else {
                    return RNODRIVER; // No driver present for the filesystem
                }
            }
        }

        return RNOENT;  // File does not exist (because filesystem doesn't exist)
    }

    uint64_t fwrite(uint64_t fd, char* buffer, uint64_t size) {
        file_t* file = get_file(fd);

        if(file == nullptr) return RINVFD; // No file present for given file descriptor

        char drive_label = file->drive_label;

        for(int i = 0; i < filesystems->size(); i++) {
            Filesystem* fs = (Filesystem*) filesystems->get(i);
            if(fs->get_drive_label() == drive_label) {
                uint16_t type = fs->get_fstype();
                
                if(type == FAT32_ID) {
                    FAT32* fat32 = (FAT32*) fs->get_filesystem();
                    // fat32->fwrite(file, buffer);
                    return RNODRIVER;
                } else if(type == RAMFS_ID) {
                    RAMFS* ramfs = (RAMFS*) fs->get_filesystem();
                    return ramfs->fwrite(file, buffer, size);
                } else {
                    return RNODRIVER; // We should never get here - if the file was opened with a driver, that driver should still be present
                }
            }
        }

        return RNOENT; // In theory, we should never reach here either
    }

    uint64_t fclose(uint64_t fd) {
        file_t* file = get_file(fd);

        if(file == nullptr) return RINVFD;

        free((void*) file->name);

        char drive_label = file->drive_label;

        for(int i = 0; i < filesystems->size(); i++) {
            Filesystem* fs = (Filesystem*) filesystems->get(i);
            if(fs->get_drive_label() == drive_label) {
                uint16_t type = fs->get_fstype();
                
                if(type == FAT32_ID) {
                    FAT32* fat32 = (FAT32*) fs->get_filesystem();
                    fat32->fclose(file);
                    return RSUCCESS;
                } else {
                    return RNODRIVER; // We should never get here - if the file was opened with a driver, that driver should still be present
                }
            }
        }

        return RNOENT; // In theory, we should never reach here either
    }

    uint64_t fstat(uint64_t fd, fstat_t* statbuf) {
        file_t* file = get_file(fd);

        if(file == nullptr) return RINVFD;

        fstat_t stat;
        stat.size = file->size;

        memcpy((void*) statbuf, (void*) &stat, sizeof(fstat_t));

        return RSUCCESS;
    }

    uint32_t new_label() {
        if(filesystems->size() == 0) return 0;

        // This code goes through the list and finds any gaps to allocate a new label in
        // If there are no gaps, we return a new label at the end
        
        uint32_t last = 0;

        if(((Filesystem*) filesystems->get(0))->get_drive_label() == 1) {
            return 0;
        }

        for(int i = 0; i < filesystems->size(); i++) {
            Filesystem* current = (Filesystem*) filesystems->get(1);
            if(last + 1 != current->get_drive_label()) {
                return last + 1;
            }
        }

        return filesystems->size();
    }

    void add_fs(Filesystem* fs) {
        // Here we find where the new driver should be inserted so the list stays sorted (based on drive label)
        if(filesystems->size() == 0) {
            filesystems->add((void*) fs);
            return;
        }

        uint32_t last = 0;

        uint32_t drive_label = fs->get_drive_label();

        if(drive_label == 0) {
            filesystems->insert(0, (void*) fs);
            return;
        }

        for(int i = 1; i < filesystems->size(); i++) {
            Filesystem* f = (Filesystem*) filesystems->get(i);
            if(last < drive_label && f->get_drive_label() > drive_label) {
                filesystems->insert(i, (void*) fs);
                return;
            }
        }

        filesystems->add((void*) fs);
    }
}

// Filesystem class

Filesystem::Filesystem(uint32_t drive_label, uint16_t fstype, void* filesystem_class) {
    this->drive_label = drive_label;
    this->type = fstype;
    this->filesystem = filesystem_class;
}

Filesystem::~Filesystem() {

}

char Filesystem::get_drive_label() {
    return this->drive_label;
}

uint16_t Filesystem::get_fstype() {
    return this->type;
}

void* Filesystem::get_filesystem() {
    return this->filesystem;
}
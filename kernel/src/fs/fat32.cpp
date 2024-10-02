#include "fat32.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mem/memory.h"
#include "../ahci/ahci.h"

#define GET_CLUSTER_FROM_ENTRY(x) (x->first_cluster_low | (x->first_cluster_high << (32 / 2)))

FAT32::FAT32(MSDOSPartitionTable* partition_table, uint8_t partition) {
    this->part_table = partition_table;

    this->partition = partition;
    this->part_table->read_partitions();
    this->partition_offset = part_table->get_mbr()->primaryPartition[partition].start_lba;

    this->bpb = read_bpb(partition_offset);

    // Calculating positions
    this->fat_start = partition_offset + bpb->reserved_sectors;
    uint32_t fat_size = bpb->table_size;

    uint32_t data_start = fat_start + fat_size * bpb->fat_copies;

    this->root_start = data_start + bpb->sectors_per_cluster * (bpb->root_cluster - 2);
}

FAT32::~FAT32() {
    delete this->part_table; // Delete the partition table

    // Delete the BPB
    memset((void*) this->bpb, 0, sizeof(BiosParameterBlock32));
    free((void*) this->bpb);
}

BiosParameterBlock32* FAT32::read_bpb(uint32_t partition_offset) {
    // Allocate a page for the data being read from the disk
    void* read_buffer = memory::get_allocator()->alloc_page();
    memset(read_buffer, 0, 0x1000);

    //ahci::driver->read(0, partition_offset, 1, (uint8_t*) read_buffer); // Read the BPB (1 sector) from the disk

    BiosParameterBlock32* bpb = (BiosParameterBlock32*) malloc(sizeof(BiosParameterBlock32));
    memcpy((void*) bpb, read_buffer, sizeof(BiosParameterBlock32));

    memset(read_buffer, 0, 0x1000);
    memory::get_allocator()->free_page((void*) read_buffer);

    return bpb;
}

file_t* FAT32::fopen(const char* filename, const char* mode) {
    file_t* file = (file_t*) malloc(sizeof(file_t));
    
    // Determining file mode
    if(strequ(mode, "r")) {
        file->mode_flags = 0b00000001;
    } else if(strequ(mode, "w")) {
        file->mode_flags = 0b00000010;
    } else if(strequ(mode, "rw")) {
        file->mode_flags = 0b00000011;
    } else {
        free((void*) file);
        return nullptr;
    }

    DirectoryEntryFat32* dirent = get_dirent(filename);

    if(dirent == nullptr) {
        free((void*) file);
        return nullptr;
    } else {
        file->size = dirent->size;
        file->dirent = (void*) dirent;
    }

    return file;
}

void FAT32::fread(file_t* file, char* buffer) {
    if((file->mode_flags & 0b00000001) == 0) // Checking if read bit is not set
        return;

    DirectoryEntryFat32* dirent = (DirectoryEntryFat32*) file->dirent;
    memset((void*) buffer, 0, dirent->size);

    uint8_t* temp_page = (uint8_t*) memory::get_allocator()->alloc_page();

    uint32_t cluster = GET_CLUSTER_FROM_ENTRY(dirent);

    bool first = true;

    uint64_t offset = 0;

    while(cluster < 0x0FFFFFF8) {
        // Calculate the sector we need to read
        uint64_t file_sector = this->root_start + bpb->sectors_per_cluster * (cluster-2);

        //ahci::driver->read(0, file_sector, 1, temp_page);

        // Copy the right amount of bytes from temp_page into the actual buffer
        size_t cpy_size = 512;

        if(file->size - offset < 512) {
            cpy_size = dirent->size - offset;
        }

        memcpy((void*) ((uint8_t*)(buffer) + offset), (void*) temp_page, cpy_size);

        // Get the next cluster
        cluster = fat_read(cluster);
        offset += 512;
    }

    memset((void*) temp_page, 0, 0x1000);

    buffer[dirent->size] = '\0';

    memory::get_allocator()->free_page((void*) temp_page);
}

void FAT32::fclose(file_t* file) {
    free(file->dirent);
    free((void*) file);
}

uint32_t FAT32::fat_read(uint32_t cluster) {
    unsigned char FAT_table[512];
    unsigned int fat_offset = cluster * 4;
    unsigned int fat_sector = fat_start + (fat_offset / 512);
    unsigned int ent_offset = fat_offset % 512;

    //ahci::driver->read(0, fat_sector, 1, (uint8_t*)(&FAT_table));

    uint32_t table_value = *(unsigned int*)&FAT_table[ent_offset];
    table_value &= 0x0FFFFFFF;

    return table_value;
}

// uint32_t FAT32::cluster_read(void* temp_page, uint32_t cluster) {

// }
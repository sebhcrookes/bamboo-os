#pragma once

#include <stdint.h>

#include <vector.h>

#include "partition_table.h"
#include "vfs.h"

// FAT32

struct BiosParameterBlock32 {
    uint8_t jump[3];
    uint8_t soft_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_copies;
    uint16_t root_dir_entries;
    uint16_t total_sectors;
    uint8_t media_type;
    uint16_t fat_sector_count;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sector_count;

    uint32_t table_size;
    uint16_t ext_flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fat_info;
    uint16_t backup_sector;
    uint8_t rsv0[12];
    uint8_t drive_number;
    uint8_t reserved;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fat_type_label[8];
}__attribute__((packed));

struct DirectoryEntryFat32 {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t c_time_tenth;
    uint16_t c_time;
    uint16_t c_date;
    uint16_t a_time;

    uint16_t first_cluster_high;

    uint16_t w_time;
    uint16_t w_date;
    uint16_t first_cluster_low;
    uint32_t size;

}__attribute__((packed));

struct LFNEntryFat32 {
    uint8_t entry_order;
    uint8_t first_chars[10]; // 5 chars
    uint8_t attribute; // Always 0x0F
    uint8_t long_entry_type;
    uint8_t checksum;
    uint8_t second_chars[12]; // 6 chars
    uint8_t reserved[2]; // Always zero
    uint8_t third_chars[4]; // 2 chars
}__attribute__((packed));

class FAT32 {
    private:
        MSDOSPartitionTable* part_table;
        BiosParameterBlock32* bpb;

        uint8_t partition;
        uint32_t partition_offset;

        uint32_t fat_start;
        uint32_t root_start;
    public:
        FAT32(MSDOSPartitionTable* partition_table, uint8_t partition);
        ~FAT32();

        BiosParameterBlock32* read_bpb(uint32_t partition_offset);

        file_t* fopen(const char* filename, const char* mode);
        void fread(file_t* file, char* buffer);
        void fclose(file_t* file);

        // 'fat32_util.cpp'
        bool exists(const char* filename);
        DirectoryEntryFat32* get_dirent(const char* filename);

        bool is_sfn_equal(char* sfname, char* sfext, char* filename);
        bool is_lfn_equal(Vector* backwards_lfn, char* filename);

    private:
        uint32_t fat_read(uint32_t cluster);
};

void read_bios_block(uint32_t partition_offset);

extern BiosParameterBlock32* g_bpb;
extern uint32_t fat_start;
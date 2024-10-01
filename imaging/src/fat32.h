#pragma once

#include <stdio.h>
#include <stdint.h>

#define MEDIA_FIXED 0xF8
#define MEDIA_REMOVABLE 0xF0

typedef struct {
    uint8_t boot_code[3];
    char oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_ent_count;
    uint16_t tot_sectors;
    uint8_t media;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t tot_sectors_32;

    /* Extended BPB values */
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t bk_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_num;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    char volume_label[11];
    char filesystem_type[9];
} __attribute__((packed)) bpb32_t;

typedef struct {
    FILE* image;
    uint64_t start_lba;
    uint64_t end_lba;
} fat32_t;

fat32_t* fat_init(FILE* file, uint64_t start_lba, uint64_t end_lba);
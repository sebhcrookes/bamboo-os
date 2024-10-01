#include "fat32.h"

#include <stdlib.h>

#include "globals.h"

void fat_write_bpb(fat32_t* fat32) {
    bpb32_t bpb = {
        .boot_code = { 0xEB, 0xFE, 0x90 },  // Not using EB 3C 90 as we just want an infinite loop
        .oem_identifier = "BAMBOOOS",
        .bytes_per_sector = lba_size,
        .sectors_per_cluster = 1,
        .reserved_sectors = 32,
        .num_fats = 2,
        .root_ent_count = 0,
        .tot_sectors = 0,
        .media = MEDIA_FIXED,
        .sectors_per_fat = 0,
        .sectors_per_track = 0,
        .num_heads = 0,
        .hidden_sectors = fat32->start_lba - 1,
        .tot_sectors_32 = fat32->end_lba - fat32->start_lba,

        /* Extended BPB values */
        .volume_label = "EFIPART    ",
        .filesystem_type = "FAT32   ",
    };

    seek_lba(fat32->image, fat32->start_lba);
    fwrite(&bpb, 1, sizeof(bpb32_t), fat32->image);
}

fat32_t* fat_init(FILE* file, uint64_t start_lba, uint64_t end_lba) {
    fat32_t* fat32 = malloc(sizeof(fat32));
    fat32->image = file;
    fat32->start_lba = start_lba;
    fat32->end_lba = end_lba;

    fat_write_bpb(fat32);

    return fat32;
}
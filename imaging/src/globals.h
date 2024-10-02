#pragma once

#include <stdint.h>
#include <stdio.h>

#define MEGABYTE 1024 * 1024
#define PARTITION_ENTRY_SIZE 128
#define NUM_GPT_TABLE_ENTRIES 128
#define GPT_TABLE_SIZE 16384
#define ALIGNMENT 1024 * 1024
#define NUM_PARTITIONS 2

extern char* image_filename;
extern uint64_t lba_size;
extern uint64_t esp_size;
extern uint64_t data_size;
extern uint64_t image_size;
extern uint64_t esp_lba_sz, data_lba_sz, image_lba_sz;    // Sizes in LBA
extern uint64_t gpt_table_lba_sz;
extern uint64_t esp_lba_st, data_lba_st;            // Start LBA value
extern uint64_t align_lba;

static uint64_t bytes_to_lbas(uint64_t bytes) {
    /*
        Returns the number of whole LBAs in the bytes, and an additional
        LBA if there is a remainder
    */
    return (bytes / lba_size) + (bytes % lba_size > 0 ? 1 : 0);
}

static void seek_lba(FILE* file, uint64_t lba) {
    fseek(file, lba * lba_size, SEEK_SET);
}

static uint64_t next_aligned_lba(uint64_t lba) {
    return lba - (lba % align_lba) + align_lba;
}
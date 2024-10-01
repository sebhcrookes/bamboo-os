#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <uchar.h>

typedef struct {
    uint32_t time_low;
    uint16_t time_mid;
    uint16_t time_high_and_ver; // Highest 4 bits are version number
    uint8_t clock_seq_high_and_res; // Highest 4 bits are variant number
    uint8_t clock_seq_low;
    uint8_t node[6];
} __attribute__((packed)) guid_t;

typedef struct {
    char signature[8]; // "EFI PART", encoded as the 64-bit constant 0x5452415020494645
    uint16_t revision_minor;
    uint16_t revision_major;
    uint32_t header_size;   // Size in bytes of the GPT header. Must be >= 92 and <= LBA size
    uint32_t header_crc32; // CRC32 checksum for the GPT header structure
    uint32_t reserved;
    uint64_t my_lba;        // LBA which contains this data structure
    uint64_t alternate_lba; // LBA address of the alternate GPT header
    uint64_t first_usable_lba;  // First usable logical block used by a partition
    uint64_t last_usable_lba;   // Last usable logical block used by a partition
    guid_t disk_guid;     // GUID that can uniquely identify the disk
    uint64_t part_table_lba;    // Starting LBA of the partition entry array
    uint32_t num_part_entries;  // Number of partition entries
    uint32_t size_part_entry;   // Size, in bytes, of each partition entry structure
    uint32_t part_table_crc32; // CRC32 of the partition entry array

    /* The rest of the block is reserved and must be zero */
} __attribute__((packed)) gpt_header_t;

typedef struct {
    guid_t partition_type;
    guid_t unique_guid;
    uint64_t start_lba;
    uint64_t end_lba;
    uint64_t attributes;
    char16_t name[36];
} __attribute__((packed)) gpt_partition_t;

bool gpt_write(FILE* file);
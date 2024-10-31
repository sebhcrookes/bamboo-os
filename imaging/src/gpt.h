#pragma once

#include <stdint.h>
#include <stdio.h>
#include <uchar.h>

#include "layout.h"

typedef struct {
    uint32_t time_lo;
    uint16_t time_mid;
    uint16_t time_hi_and_version; // Highest 4 bits are version number
    uint8_t clock_seq_hi_and_reserved; // Highest 4 bits are variant
    uint8_t clock_seq_lo;
    uint8_t node[6]; // The spatially unique node identifier (I use the MAC address)
} __attribute__((packed)) guid_t;

typedef struct {
    char signature[8]; // "EFI PART", encoded as the 64-bit constant 0x5452415020494645
    uint16_t revision_minor;
    uint16_t revision_major;
    uint32_t header_size;   // Size in bytes of the GPT header. Must be >= 92 and <= LBA size
    uint32_t header_crc32; // CRC32 checksum for the GPT header structure
    uint32_t reserved;
    uint64_t this_lba;        // LBA which contains this data structure
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

class GPT {
private:
    gpt_header_t* m_header;
    gpt_header_t* m_secondary_header;

    gpt_partition_t* m_partition_table;

    Layout& m_layout;
    size_t m_image_size_lba;
    size_t m_partition_table_size_lba;
    
public:
    GPT(Layout layout);
    ~GPT();

    void write(FILE* file);

    void calculate_crcs();
};

guid_t generate_guid();
uint32_t crc32(uint8_t* structure, uint32_t length);
#pragma once

#include <stdio.h>
#include <stdint.h>

#include "layout.h"

/* 
    The first sector on a GPT disk holds a protective MBR. This is designed
    to stop tools which only support MBR disks from misrecognising
    and overwriting GPT disks (and for a bit of backward compatibility)
*/

typedef struct {
    uint8_t boot_indicator;
    uint8_t start_head;
    uint8_t start_sector;
    uint8_t start_track;
    uint8_t os_indicator;
    uint8_t end_head;
    uint8_t end_sector;
    uint8_t end_track;
    uint32_t starting_lba;
    uint32_t size_lba;
} __attribute__((packed)) mbr_partition_t;

typedef struct {
    uint8_t bootstrap_code[440];    // Unused by UEFI systems
    uint32_t optional_signature;  // Unused (should be zero)
    uint16_t reserved;               // Unused (should be zero)
    mbr_partition_t partition_table[4];   // Table of four MBR partition entry (first reserves space on disk, all others set to zero)
    uint16_t signature;             // Set to 0xAA55
} __attribute__((packed)) mbr_t;

class MBR {
private:
    mbr_t* m_mbr;

    Layout& m_layout;
public:
    MBR(Layout layout);
    ~MBR();

    void write(FILE* file);
};
#include "mbr.h"

#include <stdio.h>

#include "globals.h"

bool mbr_write(FILE* file) {
    mbr_t mbr = {
        .bootstrap_code = {0},
        .unique_mbr_signature = 0,
        .unknown = 0,
        .partition[0] = {
            .boot_indicator = 0,
            .start_head = 0x00,
            .start_sector = 0x02,
            .start_track = 0x00,
            .os_indicator = 0xEE, // 0xEE for GPT protective (0xEF for UEFI system partition)
            .end_head = 0xFF,
            .end_sector = 0xFF,
            .end_track = 0xFF,
            .starting_lba = 0x00000001, // The LBA of the GPT partition header
            .size_in_lba = (uint32_t)(image_lba_sz == 0xFFFFFFFF ? 0xFFFFFFFF : image_lba_sz - 1),

        },
        .signature = 0xAA55,
    };

    seek_lba(file, 0);

    if(fwrite(&mbr, 1, sizeof(mbr_t), file) != sizeof(mbr_t)) {
        return false;
    }

    return true;
}
#include "gpt.h"

#include "globals.h"

#include <stdlib.h>

const guid_t ESP_GUID = {0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B} };
const guid_t DATA_GUID = {0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7} };

uint32_t crc32(uint8_t* structure, uint32_t length) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i < length) {
      byte = structure[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times.
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i++;
   }
   return ~crc;
}

guid_t generate_guid() {
    uint8_t rand_arr[16];

    for(uint8_t i = 0; i < sizeof(rand_arr); i++) {
        rand_arr[i] = rand() % (UINT8_MAX + 1);
    }

    guid_t gen = {
        .time_low = *(uint32_t*)&rand_arr[0],
        .time_mid = *(uint16_t*)&rand_arr[4],
        .time_high_and_ver = *(uint16_t*)&rand_arr[6],
        .clock_seq_high_and_res = rand_arr[8],
        .clock_seq_low = rand_arr[9],
        .node = {rand_arr[10], rand_arr[11], rand_arr[12], rand_arr[13], rand_arr[14], rand_arr[15]},
    };

    gen.time_high_and_ver &= ~(1 << 15);
    gen.time_high_and_ver |= (1 << 14);
    gen.time_high_and_ver &= ~(1 << 13);
    gen.time_high_and_ver &= ~(1 << 12);

    gen.clock_seq_high_and_res |= (1 << 7);
    gen.clock_seq_high_and_res |= (1 << 6);
    gen.clock_seq_high_and_res &= ~(1 << 5);

    return gen;
}

bool gpt_write(FILE* file) {
    seek_lba(file, 1);

    gpt_header_t header = {
        .signature = "EFI PART",
        .revision_minor = 0x0000,
        .revision_major = 0x0001,
        .header_size = sizeof(gpt_header_t),
        .header_crc32 = 0,
        .reserved = 0,
        .my_lba = 1,
        .alternate_lba = image_lba_sz - 1,
        .first_usable_lba = 2 + gpt_table_lba_sz,
        .last_usable_lba = image_lba_sz - 1 - gpt_table_lba_sz - 1,
        .disk_guid = generate_guid(),
        .part_table_lba = 2,
        .num_part_entries = NUM_GPT_TABLE_ENTRIES,
        .size_part_entry = PARTITION_ENTRY_SIZE,
        .part_table_crc32 = 0,
    };

    gpt_partition_t table[NUM_GPT_TABLE_ENTRIES] = {
        { // EFI System Partition
            .partition_type = ESP_GUID,
            .unique_guid = generate_guid(),
            .start_lba = esp_lba_st,
            .end_lba = esp_lba_st + esp_lba_sz,
            .attributes = 0,
            .name = u"EFI SYSTEM PARTITION",
        },

        { // Basic data partition (will have FAT32)
            .partition_type = DATA_GUID,
            .unique_guid = generate_guid(),
            .start_lba = data_lba_st,
            .end_lba = data_lba_st + data_lba_sz,
            .attributes = 0,
            .name = u"BASIC DATA PARTITION",
        },
    };

    header.part_table_crc32 = crc32((uint8_t*)&table, sizeof(table));
    header.header_crc32 = crc32((uint8_t*)&header, sizeof(gpt_header_t));

    if(fwrite(&header, 1, sizeof(gpt_header_t), file) != sizeof(gpt_header_t)) {
        return false;
    }

    seek_lba(file, 2);

    if(fwrite(&table, 1, sizeof(table), file) != sizeof(table)) {
        return false;
    }

    /* Secondary GPT header + table */

    gpt_header_t secondary = header;
    secondary.header_crc32 = 0;
    secondary.my_lba = image_lba_sz - 1;
    secondary.alternate_lba = header.my_lba;
    secondary.part_table_lba = image_lba_sz - 1 - gpt_table_lba_sz;

    secondary.header_crc32 = crc32((uint8_t*)&secondary, sizeof(gpt_header_t));

    seek_lba(file, image_lba_sz - 1 - gpt_table_lba_sz);

    if(fwrite(&table, 1, sizeof(table), file) != sizeof(table)) {
        return false;
    }

    seek_lba(file, image_lba_sz - 1);

    if(fwrite(&secondary, 1, sizeof(gpt_header_t), file) != sizeof(gpt_header_t)) {
        return false;
    }

    return true;
}
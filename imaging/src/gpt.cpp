#include "gpt.h"

#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <chrono>
#include <ctime>

// These values can be found easily online
const guid_t ESP_GUID = {0xC12A7328, 0xF81F, 0x11D2, 0xBA, 0x4B, {0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B} };
const guid_t DATA_GUID = {0xEBD0A0A2, 0xB9E5, 0x4433, 0x87, 0xC0, {0x68, 0xB6, 0xB7, 0x26, 0x99, 0xC7} };

GPT::GPT(Layout layout) : m_layout(layout) {

    m_image_size_lba = layout.get_image_size();
    m_partition_table_size_lba = (GPT_TABLE_ENTRIES * sizeof(gpt_partition_t)) / layout.get_lba_size();

    /* Creating GPT header */
    m_header = new gpt_header_t();

    memcpy((void*) m_header->signature, (void*) "EFI PART", 8);
    m_header->revision_minor = 0x0000;
    m_header->revision_major = 0x0001;
    m_header->header_size = sizeof(gpt_header_t);
    m_header->header_crc32 = 0;
    m_header->reserved = 0;
    m_header->this_lba = 1;
    m_header->alternate_lba = m_image_size_lba - 1;
    m_header->first_usable_lba = 2 + m_partition_table_size_lba;
    m_header->last_usable_lba = m_image_size_lba - m_partition_table_size_lba - 2;
    m_header->disk_guid = generate_guid();
    m_header->part_table_lba = 2;
    m_header->num_part_entries = GPT_TABLE_ENTRIES;
    m_header->size_part_entry = sizeof(gpt_partition_t);

    /* Creating the partition table and adding partitions */

    m_partition_table = (gpt_partition_t*) calloc(GPT_TABLE_ENTRIES, sizeof(gpt_partition_t));

    // The partitions are aligned on 1MB boundaries

    m_partition_table[0] = { // EFI System Partition
        .partition_type = ESP_GUID,
        .unique_guid = generate_guid(),
        .start_lba = layout.get_esp_start(),
        .end_lba = layout.get_esp_start() + layout.get_esp_size(),
        .attributes = 0,
        .name = u"EFI SYSTEM PARTITION",
    };

    m_partition_table[1] = { // Basic data partition (will have FAT32)
        .partition_type = DATA_GUID,
        .unique_guid = generate_guid(),
        .start_lba = layout.get_mainp_start(),
        .end_lba = layout.get_mainp_start() + layout.get_mainp_size(),
        .attributes = 0,
        .name = u"BASIC DATA PARTITION",
    };

    /* Create secondary GPT header */

    m_secondary_header = new gpt_header_t();
    memcpy((void*) m_secondary_header, (void*) m_header, sizeof(gpt_header_t));

    m_secondary_header->header_crc32 = 0;
    m_secondary_header->this_lba = layout.get_image_size() - 1;
    m_secondary_header->alternate_lba = m_header->this_lba;
    m_secondary_header->part_table_lba = layout.get_image_size() - 1 - m_partition_table_size_lba;

    /* Calculate CRC values for the partition table and the headers */

    this->calculate_crcs();
}

GPT::~GPT() {
    delete m_header;
    delete m_secondary_header;
    delete m_partition_table;
}

void GPT::write(FILE* file) {

    /* Write the primary header */
    m_layout.goto_lba(file, 1);
    fwrite(m_header, 1, sizeof(gpt_header_t), file);

    /* Write the primary partition table */
    m_layout.goto_lba(file, 2);
    size_t partition_table_size = sizeof(gpt_partition_t) * GPT_TABLE_ENTRIES;
    fwrite(m_partition_table, 1, partition_table_size, file);

    /* Write the secondary header */
    m_layout.goto_lba(file, m_image_size_lba - 1);
    fwrite(m_secondary_header, 1, sizeof(gpt_header_t), file);

    /* Write the secondary partition table */
    m_layout.goto_lba(file, m_image_size_lba - 1 - m_partition_table_size_lba);
    fwrite(m_partition_table, 1, partition_table_size, file);
}

void GPT::calculate_crcs() {
    // Calculate partition table checksum
    m_header->part_table_crc32 = crc32((uint8_t*)m_partition_table, sizeof(gpt_partition_t) * GPT_TABLE_ENTRIES);    
    // The primary partition table is the same as the secondary partition table
    m_secondary_header->part_table_crc32 = m_header->part_table_crc32;

    // Calculate primary header checksum
    m_header->header_crc32 = crc32((uint8_t*)m_header, sizeof(gpt_header_t));
    // Calculate the secondary header checksum
    m_secondary_header->header_crc32 = crc32((uint8_t*)m_secondary_header, sizeof(gpt_header_t));
}

guid_t generate_guid() {
    // How to make a GUID: https://guid.one/guid/make
    guid_t guid;

    /* === Getting the MAC address === */

    std::string mac_address;

    char buffer[18] = {0};
    FILE* pf = popen("cat /sys/class/net/eth0/address", "r");
    fgets(buffer, 18, pf);
    pclose(pf);

    for(int i = 0; i < 17; i++) {
        mac_address += buffer[i];
    }

     // Remove the colons and convert to an integer
    mac_address.erase(std::remove(mac_address.begin(), mac_address.end(), ':'), mac_address.end());
    uint64_t mac = strtoul(mac_address.c_str(), NULL, 16);

    /* Putting MAC address into struct */
    for(int i = 0; i < 6; i++) {
        guid.node[5 - i] = mac & 0xFF;
        mac >>= 8;
    }

    /* Randomly generating clock sequence */
    uint16_t clock_sequence = rand() % __UINT16_MAX__;
    uint8_t clock_lo = clock_sequence & 0xFF;
    uint8_t clock_hi = clock_sequence >> 8;

    guid.clock_seq_lo = clock_lo;
    guid.clock_seq_hi_and_reserved = clock_hi;

    /* Setting variant */
    guid.clock_seq_hi_and_reserved &= ~(1 << 5);
    guid.clock_seq_hi_and_reserved |= (1 << 6);
    guid.clock_seq_hi_and_reserved |= (1 << 7);

    /* Getting time and loading into struct */
    // The specification wants a timestamp of "the number of 100 nanosecond intervals since 15 October 1582 (adoption of the Gregorian calendar)"
    // I calculate this as the number of 100ns intervals to 1970, and then since.

    uint64_t unix_timestamp_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    uint64_t timestamp = 12219289125 * 10000000 + unix_timestamp_ns / 100;
    
    guid.time_lo = timestamp & 0xFFFFFFFF;
    guid.time_mid = (timestamp >> 32) & 0xFFFF;
    guid.time_hi_and_version = (timestamp >> 48) & 0xFFFF;

    /* Setting version (version 4) */
    guid.time_hi_and_version &= ~(1 << 12);
    guid.time_hi_and_version &= ~(1 << 13);
    guid.time_hi_and_version |=  (1 << 14);
    guid.time_hi_and_version &= ~(1 << 15);

    return guid;
}

// Not my code - sourced from https://web.archive.org/web/20180406173902/http://www.hackersdelight.org/hdcodetxt/crc.c.txt
uint32_t crc32(uint8_t* structure, uint32_t length) {
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (i < length) {
      byte = structure[i];
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i++;
   }
   return ~crc;
}

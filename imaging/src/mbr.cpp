#include "mbr.h"

#include <stdio.h>
#include <cstring>

MBR::MBR(Layout layout) : m_layout(layout) {
    m_mbr = new mbr_t();

    memset((void*) m_mbr, 0, sizeof(mbr_t)); // Initialise the mbr values as 0
    m_mbr->optional_signature = 0;
    m_mbr->reserved = 0;

    m_mbr->partition_table[0].boot_indicator = 0;
    m_mbr->partition_table[0].start_head = 0x00;
    m_mbr->partition_table[0].start_sector = 0x02;
    m_mbr->partition_table[0].start_track = 0x00;
    m_mbr->partition_table[0].os_indicator = 0xEE; // 0xEE for GPT protective (0xEF for UEFI system partition)
    m_mbr->partition_table[0].end_head = 0xFF;
    m_mbr->partition_table[0].end_sector = 0xFF;
    m_mbr->partition_table[0].end_track = 0xFF;
    m_mbr->partition_table[0].starting_lba = 0x00000001; // The LBA of the GPT partition header
    m_mbr->partition_table[0].size_lba = (uint32_t) std::min(m_mbr->partition_table[0].size_lba, 0xFFFFFFFF);
    
    m_mbr->signature = 0xAA55;
}

MBR::~MBR() {
    delete m_mbr;
}

void MBR::write(FILE* file) {
    m_layout.goto_lba(file, 0);
    fwrite(m_mbr, 1, sizeof(mbr_t), file);
}
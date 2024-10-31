#pragma once

#include <stdint.h>
#include <cmath>

#define PARTITIONS 2
#define GPT_TABLE_ENTRIES 128
#define GPT_TABLE_SIZE 16384 // in bytes
#define MEGABYTE 1024 * 1024

class Layout {
private:
    size_t m_lba_size;

    size_t m_esp_size_lba;
    size_t m_mainp_size_lba;
    size_t m_image_size_lba;

    uint64_t m_esp_start_lba;
    uint64_t m_mainp_start_lba;
public:
    Layout(size_t lba_size, size_t esp_size, size_t mainp_size) {
        m_lba_size = lba_size;

        m_esp_size_lba = bytes_to_lbas(esp_size);
        m_mainp_size_lba = bytes_to_lbas(mainp_size);

        m_esp_start_lba = MEGABYTE / m_lba_size; // The LBA for the ESP's start, as partitions are aligned on 1MB boundaries
        
        // The main partition will follow on from the ESP (however, it will still be aligned to 1MB)
        uint64_t esp_end_lba = m_esp_start_lba + m_esp_size_lba;
        m_mainp_start_lba = get_next_aligned_1mb(esp_end_lba);

        /* Calculating the image size */
        size_t gpt_tables_size = (GPT_TABLE_SIZE / lba_size) * 2; // There are two GPT tables - primary and secondary (backup)
        size_t partition_padding = (MEGABYTE * PARTITIONS) / m_lba_size; // Each partition has 1MB of padding after it
        size_t headers_size = 1 + 2; // 1 MBR lba and 2 GPT headers (each 1 lba)

        m_image_size_lba = m_esp_size_lba + m_mainp_size_lba + partition_padding + gpt_tables_size + headers_size;
    }

    ~Layout() {}

    size_t get_lba_size() {
        return m_lba_size;
    }

    size_t get_esp_size() {
        return m_esp_size_lba;
    }

    size_t get_mainp_size() {
        return m_mainp_size_lba;
    }

    size_t get_image_size() {
        return m_image_size_lba;
    }


    uint64_t get_esp_start() {
        return m_esp_start_lba;
    }

    uint64_t get_mainp_start() {
        return m_mainp_start_lba;
    }

    void goto_lba(FILE* file, uint64_t lba) {
        fseek(file, lba * m_lba_size, SEEK_SET);
    }

private:
    size_t bytes_to_lbas(size_t bytes) {
        /* Returns the number of LBAs that many bytes would consume */
        return (size_t) ceil((double)bytes / (double)m_lba_size);
    }

    uint64_t get_next_aligned_1mb(uint64_t lba) {
        return (MEGABYTE + ceil((lba * m_lba_size) / MEGABYTE)) / m_lba_size;
    }
};
#pragma once

#include <stdint.h>

struct PartitionTableEntry {
    uint8_t bootable; // 0x00 = not bootable, 0x80 = bootable

    uint8_t start_head;
    uint8_t start_sector : 6;
    uint16_t start_cylinder : 10;

    uint8_t partition_id; // https://en.wikipedia.org/wiki/Partition_type

    uint8_t end_head;
    uint8_t end_sector : 6;
    uint16_t end_cylinder : 10;

    uint32_t start_lba;
    uint32_t length;
}__attribute__((packed));

struct MasterBootRecord {
    uint8_t bootloader[440];
    uint32_t signature;
    uint16_t unused; // Usually null (0x0000)

    PartitionTableEntry primaryPartition[4];
    uint16_t magic_number;
}__attribute__((packed));

class MSDOSPartitionTable {
    private:
        uint32_t stor_uuid;
        MasterBootRecord* mbr;
    public:
        MSDOSPartitionTable(uint32_t storage_uuid);
        ~MSDOSPartitionTable();

        /* Reads the MBR from the disk. Returns if successful or not */
        bool read_partitions();

        MasterBootRecord* get_mbr();
};
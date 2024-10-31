#include "partition_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <faults.h>
#include <memory.h>

#include "storage.h"
#include "../ahci/ahci.h"

MSDOSPartitionTable::MSDOSPartitionTable(uint32_t storage_uuid) {
    this->stor_uuid = storage_uuid;
    this->mbr = (MasterBootRecord*) malloc(sizeof(MasterBootRecord)); // Allocate memory for the MBR
}

MSDOSPartitionTable::~MSDOSPartitionTable() {
    free((void*) this->mbr); // Free the MBR
}

bool MSDOSPartitionTable::read_partitions() {
    MasterBootRecord boot_record;

    storage::read(stor_uuid, 0, 1, (char*) &boot_record); // Read the MBR

    if(boot_record.magic_number != 0xAA55) { // Check the MBR's magic value to see if it is valid
        io::print_drv_name("FAT32");
        io::printf("Illegal MBR! Found incorrect magic number in master boot record: %X\n", boot_record.magic_number);
        return false;
    }

    memcpy((void*) this->mbr, (void*) &boot_record, sizeof(MasterBootRecord)); // Copy the MBR into the class' member variable

    return true;
}

MasterBootRecord* MSDOSPartitionTable::get_mbr() {
    return this->mbr;
}
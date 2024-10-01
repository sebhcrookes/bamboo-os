#pragma once

#include "acpi.h"

namespace acpi {

    struct config_space_base_addr_t {
        uint64_t base_address;
        uint16_t pci_segment_group_num;
        uint8_t start_pci_bus;
        uint8_t end_pci_bus;
        uint32_t reserved;
    } __attribute__((packed));

    void parse_mcfg();

}
#pragma once

#include <stdint.h>
#include <vector.h>

#include "../acpi/acpi.h"
#include "../acpi/mcfg.h"

namespace pci {
    struct device_header_t {
        uint16_t vendor_id;
        uint16_t device_id;
        uint16_t command;
        uint16_t status;
        uint8_t revision_id;
        uint8_t program_interface;
        uint8_t subclass;
        uint8_t _class;
        uint8_t cache_line_size;
        uint8_t latency_timer;
        uint8_t header_type;
        uint8_t bist;
    } __attribute__((packed));

    struct device_t {
        device_header_t* device_header;
        const char* device_name;
        const char* vendor_name;
        const char* device_class;
    };

    struct pci_header_0_t {
        device_header_t header;
        uint32_t BAR0;
        uint32_t BAR1;
        uint32_t BAR2;
        uint32_t BAR3;
        uint32_t BAR4;
        uint32_t BAR5;
        uint32_t cardbus_cis_ptr;
        uint16_t subsystem_vendor_id;
        uint16_t subsystem_id;
        uint32_t expansion_rom_base_addr;
        uint8_t capabilities_ptr;
        uint8_t reserved[7];
        uint8_t interrupt_line;
        uint8_t interrupt_pin;
        uint8_t min_grant;
        uint8_t max_latency;
    } __attribute__((packed));

    struct device_config_t {
        uint64_t base_address;
        uint16_t pci_seg_group_num;
        uint8_t start_bus_num;
        uint8_t end_bus_num;
        uint8_t reserved[4];
    } __attribute__((packed));

    void init();

    void scan_pci(acpi::mcfg_hdr_t* mcfg);
    void scan_bus(uint64_t base_address, uint64_t bus);
    void scan_device(uint64_t bus_address, uint64_t device);
    void scan_function(uint64_t device_address, uint64_t function);

    void pci_probe();

    const char* get_vendor_name(uint16_t vendor_id);
    const char* get_device_name(uint16_t vendor_id, uint16_t device_id);
    const char* get_device_class_name(uint16_t class_id);

    Vector* get_devices();
}
#include "pci.h"

#include <stdio.h>
#include <stdlib.h>

namespace pci {

    Vector* devices;

    int device_count = 0;

    #define DEVICES_PER_BUS 32
    #define FUNCTIONS_PER_DEVICE 8

    void init() {
        devices = new Vector();

        /* Try to get the MCFG table (stores info about PCI) */
        acpi::mcfg_hdr_t* mcfg = acpi::get_mcfg();

        if(mcfg != nullptr) { // "Memory Mapped PCI Configuration Space Access" - ECAM
            scan_pci(mcfg);
        } else { // "Configuration Space Access Mechanism #1" - CAM
            pci_probe();
        }
    }

    /* Memory Mapped PCI Enhanced Configuration Access Mechanism - ECAM */

    void scan_pci(acpi::mcfg_hdr_t* mcfg) {
        io::print_drv_name("PCI");
        io::printf("Enumating buses and devices... (ECAM)\n");

        int length_entries = mcfg->header.length - sizeof(acpi::mcfg_hdr_t);
        int num_entries = length_entries / sizeof(device_config_t);

        device_config_t* entries = (device_config_t*)((void*)mcfg + sizeof(acpi::mcfg_hdr_t));

        for(int i = 0; i < num_entries; i++) {
            device_config_t device_config = entries[i];

            for(uint64_t bus = device_config.start_bus_num; bus < device_config.end_bus_num; bus++) {
                scan_bus(device_config.base_address, bus);
            }
        }
    }

    /* physical_address = MMIO_starting_physical_address + (bus << 20 | device << 15 | function << 12)
                        = MMIO_starting_physical_address + (bus << 20) + (device << 15) + (function << 12)
                        
       We break up the calculation like this in order to ensure all pages which need identity mapping are identity mapped */

    void scan_bus(uint64_t base_address, uint64_t bus) {
        uint64_t bus_address = base_address + (bus << 20);
        map_memory((void*) bus_address, (void*) bus_address);

        device_header_t* device_header = (device_header_t*) bus_address;
        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) return;

        for(uint64_t device = 0; device < DEVICES_PER_BUS; device++) {
            scan_device(bus_address, device);
        }
    }

    void scan_device(uint64_t bus_address, uint64_t device) {
        uint64_t device_address = bus_address + (device << 15);
        map_memory((void*) device_address, (void*) device_address);

        device_header_t* device_header = (device_header_t*) device_address;
        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) return;

        for(uint64_t function = 0; function < FUNCTIONS_PER_DEVICE; function++) {
            scan_function(device_address, function);
        }
    }

    void scan_function(uint64_t device_address, uint64_t function) {
        uint64_t function_address = device_address + (function << 12);
        map_memory((void*) function_address, (void*) function_address);

        device_header_t* device_header = (device_header_t*) function_address;

        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) return;

        // Creating a new device struct to store a pointer to the PCI device header and information about the device
        device_t* dev = (device_t*) malloc(sizeof(device_t));
        dev->device_header = device_header;
        dev->vendor_name = get_vendor_name(device_header->vendor_id);
        dev->device_name = get_device_name(device_header->vendor_id, device_header->device_id);
        dev->device_class = get_device_class_name(device_header->_class);

        // Printing the information about the device
        io::set_print_colour(COL_SPECIAL_PRINT);
        io::printf(" [%u]", device_count);
        io::set_print_colour(COL_PRINT);
        io::printf(": %s", dev->vendor_name);
        io::printf(" - %s\n", dev->device_class);

        // Add this device to the list
        devices->add((void*) dev);

        device_count++;
    }

    /* PCI Configuration Space Access Mechanism #1 - CAM */

    uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
        uint32_t address = 0;

        address |= (uint32_t)bus << 16;
        address |= (uint32_t)slot << 11;
        address |= (uint32_t)func << 8;
        address |= (uint32_t)offset & 0xFC;
        address |= (uint32_t)0x80000000;

        io::io_write_32(0xCF8, address);
    
        return (uint16_t)(io::io_read_32(0xCFC) >> ((offset & 2) * 8));
    }

    void get_device_header(device_header_t* device_header, uint16_t bus, uint16_t device, uint16_t function) {
        uint16_t* hdr = (uint16_t*) device_header;

        for(int i = 0; i < 16; i += 2) { /// 16 is the length of device header in bytes
            uint16_t val = pci_read_word(bus, device, function, i);
            hdr[i / 2] = val;
        }
    }

    void pci_probe() {
        io::print_drv_name("PCI");
        io::printf("Enumating buses and devices... (CAM)\n");

        // Probing by brute force, trying every possible bus, slot and function
        for(uint32_t bus = 0; bus < 256; bus++) {
            for(uint32_t slot = 0; slot < DEVICES_PER_BUS; slot++) {
                for(uint32_t function = 0; function < FUNCTIONS_PER_DEVICE; function++) {
                    uint16_t vendor_id = pci_read_word(bus, slot, function, 0); // Quick way to access the vendor ID (without calling get_device_header)
                    if(vendor_id == 0xFFFF) continue;

                    device_header_t* hdr = (device_header_t*) malloc(sizeof(device_header_t));
                    get_device_header(hdr, bus, slot, function);

                    device_t* dev = (device_t*) malloc(sizeof(device_t));
                    dev->device_header = hdr;
                    dev->vendor_name = get_vendor_name(hdr->vendor_id);
                    dev->device_name = get_device_name(hdr->vendor_id, hdr->device_id);
                    dev->device_class = get_device_class_name(hdr->_class);

                    // Printing the information about the device
                    io::set_print_colour(COL_SPECIAL_PRINT);
                    io::printf(" [%u]", device_count);
                    io::set_print_colour(COL_PRINT);
                    io::printf(": %s", dev->vendor_name);
                    io::printf(" - %s\n", dev->device_class);

                    // Add this device to the list
                    devices->add((void*) dev);

                    device_count++;
                }
            }
        }
    }


    Vector* get_devices() {
        return devices;
    }
}
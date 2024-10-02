#include "pci.h"

#include <stdio.h>
#include <stdlib.h>

namespace pci {

    Vector* devices;

    int device_count = 0;

    void init() {
        devices = new Vector();

        /* Try to get the MCFG table (stores info about PCI) */
        acpi::mcfg_hdr_t* mcfg = acpi::get_mcfg();

        if(mcfg != nullptr) { // "Memory Mapped PCI Configuration Space Access" - ECAM
            enumerate_pci(mcfg);
        } else { // "Configuration Space Access Mechanism #1" - CAM
            pci_probe();
        }
    }

    /* Memory Mapped PCI Enhanced Configuration Access Mechanism - ECAM */

    void enumerate_function(uint64_t device_address, uint64_t function) {
        uint64_t offset = function << 12;

        uint64_t function_address = device_address + offset;
        map_memory((void*) function_address, (void*) function_address);

        device_header_t* device_header = (device_header_t*) function_address;

        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) {
            return;
        }

        // Creating a new device struct to store a pointer to the PCI device header and information about the device
        device_t* dev = (device_t*) malloc(sizeof(device_t));
        dev->device_header = device_header;
        dev->vendor_name = get_vendor_name(device_header->vendor_id);
        dev->device_name = get_device_name(device_header->vendor_id, device_header->device_id);
        dev->device_class = get_device_name(device_header->vendor_id, device_header->device_id);
        dev->subclass_name = get_subclass_name(device_header->_class, device_header->subclass);
        dev->program_interface_name = get_program_interface_name(device_header->_class, device_header->subclass, device_header->program_interface);

        // Printing the information about the device

        io::printf(" [%u]: %s", device_count, dev->vendor_name);

        io::printf(" - %s | %s / %s\n",
        dev->device_class,
        dev->subclass_name,
        dev->program_interface_name);

        // Add this device to the list
        devices->add((void*) dev);

        device_count++;
    }

    void enumerate_device(uint64_t bus_address, uint64_t device) {
        uint64_t offset = device << 15;

        uint64_t device_address = bus_address + offset;
        map_memory((void*) device_address, (void*) device_address);

        device_header_t* device_header = (device_header_t*) device_address;

        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) return;

        for(uint64_t function = 0; function < 8; function++) {
            enumerate_function(device_address, function);
        }
    }

    void enumerate_bus(uint64_t base_address, uint64_t bus) {
        uint64_t offset = bus << 20;

        uint64_t bus_address = base_address + offset;
        map_memory((void*) bus_address, (void*) bus_address);

        device_header_t* device_header = (device_header_t*) bus_address;

        if(device_header->device_id == 0 || device_header->device_id == 0xFFFF) return;

        for(uint64_t device = 0; device < 32; device++) {
            enumerate_device(bus_address, device);
        }
    }

    void enumerate_pci(acpi::mcfg_hdr_t* mcfg) {
        io::print_drv_name("PCI");
        io::printf("Enumating buses and devices... (ECAM)\n");

        int entries = ((mcfg->header.length) - sizeof(acpi::mcfg_hdr_t)) / sizeof(device_config_t);

        for(int i = 0; i < entries; i++) {
            device_config_t* new_device_config = (device_config_t*)((uint64_t) mcfg + sizeof(acpi::mcfg_hdr_t) + (sizeof(device_config_t) * i));

            for(uint64_t bus = new_device_config->start_bus; bus < new_device_config->end_bus; bus++) {
                enumerate_bus(new_device_config->base_address, bus);
            }
        }
    }

    /* PCI Configuration Space Access Mechanism #1 - CAM */

    uint16_t pci_read_word(uint16_t bus, uint16_t slot, uint16_t func, uint16_t offset) {
        uint64_t address;
        uint64_t lbus = (uint64_t)bus;
        uint64_t lslot = (uint64_t)slot;
        uint64_t lfunc = (uint64_t)func;
        uint16_t tmp = 0;
        address = (uint64_t)((lbus << 16) | (lslot << 11) |
                (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));
        io::io_write_32 (0xCF8, address);
        tmp = (uint16_t)((io::io_read_32 (0xCFC) >> ((offset & 2) * 8)) & 0xffff);
        return (tmp);
    }

    void get_device_header(device_header_t* device_header, uint16_t bus, uint16_t device, uint16_t function) {
        uint16_t* hdr = (uint16_t*) device_header;

        for(int i = 0; i < 16; i += 2) { /// 16 is the length of device header in bytes
            uint16_t val = pci_read_word(bus, device, function, i);
            hdr[i / 2] = val;
        }
    }

    void pci_probe() {
        io::printf("PCI", "Enumating buses and devices... (CAM)\n");
        for(uint32_t bus = 0; bus < 0xFF; bus++) {
            for(uint32_t slot = 0; slot < 0x20; slot++) {
                for(uint32_t function = 0; function < 8; function++) {
                    uint16_t vendor_id = pci_read_word(bus, slot, function, 0); // Quick way to access the vendor ID (without calling get_device_header)
                    if(vendor_id == 0xFFFF) continue;

                    device_header_t* hdr = (device_header_t*) malloc(sizeof(device_header_t));
                    get_device_header(hdr, bus, slot, function);

                    device_t* dev = (device_t*) malloc(sizeof(device_t));
                    dev->device_header = hdr;
                    dev->vendor_name = get_vendor_name(hdr->vendor_id);
                    dev->device_name = get_device_name(hdr->vendor_id, hdr->device_id);
                    dev->device_class = get_device_name(hdr->vendor_id, hdr->device_id);
                    dev->subclass_name = get_subclass_name(hdr->_class, hdr->subclass);
                    dev->program_interface_name = get_program_interface_name(hdr->_class, hdr->subclass, hdr->program_interface);

                    // Printing the information about the device

                    io::printf(" [%u]: %s", device_count, dev->vendor_name);

                    io::printf(" - %s | %s / %s\n",
                    dev->device_class,
                    dev->subclass_name,
                    dev->program_interface_name);

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
#include "pci_drivers.h"

#include <vector.h>
#include <stdio.h>

#include "../ahci/ahci.h"

namespace pci {
    void initialise_pci_drivers() {
        Vector* devices = get_devices();
        for(int i = 0; i < devices->size(); i++) {
            device_t* device = (device_t*) devices->get(i);
            device_header_t* device_header = device->device_header;

            switch (device_header->_class) {
                case 0x01: { // Mass storage controller
                    switch (device_header->subclass) {
                        case 0x06: { // Serial ATA 
                            switch (device_header->program_interface) {
                                case 0x01: { // AHCI 1.0 Device
                                    //ahci::AHCIDriver* ahci_driver = new ahci::AHCIDriver(device);
                                    io::print_drv_name("AHCI");
                                    io::printf("AHCI driver not yet installed (FAT32 code present)\n");
                                } break;
                            }
                        } break;
                    }
                } break;
                
                case 0x0C: { // Serial Bus Controller
                    switch(device_header->subclass) {
                        case 0x03: { // USB Controller
                            switch(device_header->program_interface) {
                                case 0x30: { // xHCI (USB3) Controller
                                    io::printf("TODO: Should initialise xHCI driver here\n");
                                } break;
                            }
                        } break;
                    }
                } break;
            }
        }
    }
}
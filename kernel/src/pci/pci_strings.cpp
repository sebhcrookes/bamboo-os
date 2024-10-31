#include "pci.h"

#include <stdint.h>

#define INTEL_VENDOR_ID     0x8086
#define AMD_VENDOR_ID       0x1022
#define NVIDIA_VENDOR_ID    0x10DE
#define REDHAT_VENDOR_ID    0x1B36
#define VM_GRAPHICS_ADAPTOR_VENDOR_ID 0x1234

namespace pci {

    /* Keep these simple, because later on we will use networking to look up the values online (probably have proper offline lookup tables too) */

    const char* get_vendor_name(uint16_t vendor_id) {
        switch (vendor_id) {
            case INTEL_VENDOR_ID:
                return "Intel Corp";
            case AMD_VENDOR_ID:
                return "Advanced Micro Devices, Inc. [AMD]";
            case NVIDIA_VENDOR_ID:
                return "NVIDIA Corporation";
            case REDHAT_VENDOR_ID:
                return "Redhat, Inc.";
            case VM_GRAPHICS_ADAPTOR_VENDOR_ID:
                return "Technical Corp.";
        }
        return "Unknown Vendor";
    }

    const char* get_device_name(uint16_t vendor_id, uint16_t device_id) {
        switch (vendor_id) {
            case INTEL_VENDOR_ID: // Intel
                switch(device_id) {
                    case 0x29C0:
                        return "Express DRAM Controller";
                    case 0x2918:
                        return "LPC Interface Controller";
                    case 0x2922:
                        return "6 port SATA Controller [AHCI mode]";
                    case 0x2930:
                        return "SMBus Controller";
                }
        }
        return "Unknown Device";
    }

    const char* get_device_class_name(uint16_t class_id) {
        char* list[] = {"Unclassified device",
            "Mass storage controller",
            "Network controller",
            "Display controller",
            "Multimedia controller",
            "Memory controller",
            "Bridge",
            "Communication controller",
            "Generic system peripheral",
            "Input device controller",
            "Docking station",
            "Processor",
            "Serial bus controller",
            "Wireless controller",
            "Intelligent controller",
            "Satellite communications controller",
            "Encryption controller",
            "Signal processing controller",
            "Processing accelerators",
            "Non-Essential Instrumentation"};
        if(class_id == 0x40) return "Coprocessor";

        if(class_id <= 0x13) return list[class_id];

        return "Unassigned class";
    }
}
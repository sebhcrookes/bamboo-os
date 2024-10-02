#include "pci.h"

#include <stdint.h>

#define INTEL_VENDOR_ID     0x8086
#define AMD_VENDOR_ID       0x1022
#define NVIDIA_VENDOR_ID    0x10DE
#define REDHAT_VENDOR_ID    0x1B36

namespace pci {
    const char* DeviceClasses[] {
        "Unclassified",
        "Mass Storage Controller",
        "Network Controller",
        "Display Controller",
        "Multimedia Controller",
        "Memory Controller",
        "Bridge Device",
        "Simple Communication Controller",
        "Base System Peripheral",
        "Input Device Controller",
        "Docking Station", 
        "Processor",
        "Serial Bus Controller",
        "Wireless Controller",
        "Intelligent Controller",
        "Satellite Communication Controller",
        "Encryption Controller",
        "Signal Processing Controller",
        "Processing Accelerator",
        "Non Essential Instrumentation"
    };

    const char* get_vendor_name(uint16_t vendor_id) {
        switch (vendor_id) {
            case INTEL_VENDOR_ID:
                return "Intel Corp";
            case AMD_VENDOR_ID:
                return "Advanced Micro Devices, Inc. [AMD]";
            case NVIDIA_VENDOR_ID:
                return "NVIDIA Corporation";
            case REDHAT_VENDOR_ID:
                return "Red Hat, Inc";
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

    const char* mass_storage_controller_subclass_name(uint8_t subclass_code) {
        switch (subclass_code) {
            case 0x00:
                return "SCSI Bus Controller";
            case 0x01:
                return "IDE Controller";
            case 0x02:
                return "Floppy Disk Controller";
            case 0x03:
                return "IPI Bus Controller";
            case 0x04:
                return "RAID Controller";
            case 0x05:
                return "ATA Controller";
            case 0x06:
                return "Serial ATA";
            case 0x07:
                return "Serial Attached SCSI";
            case 0x08:
                return "Non-Volatile Memory Controller";
            case 0x80:
                return "Other";
        }
        return "Unknown";
    }

    const char* serial_bus_controller_subclass_name(uint8_t subclass_code) {
        switch (subclass_code) {
            case 0x00:
                return "FireWire (IEEE 1394) Controller";
            case 0x01:
                return "ACCESS Bus";
            case 0x02:
                return "SSA";
            case 0x03:
                return "USB Controller";
            case 0x04:
                return "Fibre Channel";
            case 0x05:
                return "SMBus";
            case 0x06:
                return "Infiniband";
            case 0x07:
                return "IPMI Interface";
            case 0x08:
                return "SERCOS Interface (IEC 61491)";
            case 0x09:
                return "CANbus";
            case 0x80:
                return "SerialBusController - Other";
        }
        return "Unknown";
    }

    const char* bridge_device_subclass_name(uint8_t subclass_code) {
        switch (subclass_code) {
            case 0x00:
                return "Host Bridge";
            case 0x01:
                return "ISA Bridge";
            case 0x02:
                return "EISA Bridge";
            case 0x03:
                return "MCA Bridge";
            case 0x04:
                return "PCI-to-PCI Bridge";
            case 0x05:
                return "PCMCIA Bridge";
            case 0x06:
                return "NuBus Bridge";
            case 0x07:
                return "CardBus Bridge";
            case 0x08:
                return "RACEway Bridge";
            case 0x09:
                return "PCI-to-PCI Bridge";
            case 0x0a:
                return "InfiniBand-to-PCI Host Bridge";
            case 0x80:
                return "Other";
        }
        return "Unknown";
    }

    const char* get_subclass_name(uint8_t class_code, uint8_t subclass_code) {
        switch (class_code) {
            case 0x01:
                return mass_storage_controller_subclass_name(subclass_code);
            case 0x03:
                switch (subclass_code) {
                    case 0x00:
                        return "VGA Compatible Controller";
                }
            case 0x06:
                return bridge_device_subclass_name(subclass_code);
            case 0x0C:
                return serial_bus_controller_subclass_name(subclass_code);
        }
        return "Unknown";
    }

    const char* get_program_interface_name(uint8_t class_code, uint8_t subclass_code, uint8_t program_interface) {
        switch (class_code) {
            case 0x01:
                switch (subclass_code) {
                    case 0x06:
                        switch (program_interface) {
                            case 0x00:
                                return "Vendor Specific Interface";
                            case 0x01:
                                return "AHCI 1.0";
                            case 0x02:
                                return "Serial Storage Bus";
                        }
                }
            case 0x03:
                switch (subclass_code) {
                    case 0x00:
                        switch (program_interface) {
                            case 0x00:
                                return "VGA Controller";
                            case 0x01:
                                return "8514-Compatible Controller";
                        }
                }
            case 0x0C:
                switch (subclass_code) {
                    case 0x03:
                        switch (program_interface) {
                            case 0x00:
                                return "UHCI Controller";
                            case 0x10:
                                return "OHCI Controller";
                            case 0x20:
                                return "EHCI (USB2) Controller";
                            case 0x30:
                                return "XHCI (USB3) Controller";
                            case 0x80:
                                return "Unspecified";
                            case 0xFE:
                                return "USB Device (Not a Host Controller)";
                        }
                }    
        }
        return "Unknown Program Interface";
    }
}
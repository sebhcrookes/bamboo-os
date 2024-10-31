#pragma once

#include <vector.h>

#include "acpi.h"

namespace acpi {

    #define LAPIC_TYPE  0
    #define IOAPIC_TYPE 1
    #define IOAPIC_INT_SRC_OVERRIDE 2

    /* Structs */
    
    struct apic_madt_t {
        sdt_hdr_t hdr;
        uint32_t local_apic_addr;
        uint32_t flags;
    } __attribute__((packed));

    struct apic_hdr_t {
        uint8_t type;
        uint8_t length;
    } __attribute__((packed));
    
    struct local_apic_t {
        apic_hdr_t hdr;
        uint8_t acpi_processor_id;
        uint8_t apic_id;
        uint32_t flags;
    } __attribute__((packed));

    struct io_apic_t {
        apic_hdr_t hdr;
        uint8_t io_apic_id;
        uint8_t reserved;
        uint32_t io_apic_addr;
        uint32_t global_system_interrupt_base;
    } __attribute__((packed));

    struct interrupt_override_t {
        apic_hdr_t hdr;
        uint8_t bus_source;
        uint8_t irq_source;
        uint32_t glob_sys_interrupt;
        uint16_t flags;
    } __attribute__((packed));

    /* Functions */

    void parse_madt();
    uint32_t madt_remap_irq(uint32_t irq);

    Vector* madt_get_lapics();
}
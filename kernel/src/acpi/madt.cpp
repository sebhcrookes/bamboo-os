#include "madt.h"

#include <stdio.h>
#include <faults.h>
#include <stdlib.h>
#include <string.h>

#include "../int/local_apic.h"
#include "../int/ioapic.h"
#include "../cpu/smp.h"

namespace acpi {

    uint64_t num_cores;

    Vector lapics;

    Vector int_overrides;

    void parse_madt() {
        apic_madt_t* madt_hdr = (apic_madt_t*) get_table("APIC"); // Calling function from acpi.h
        faults::assert(is_valid_sdt_checksum(&madt_hdr->hdr), "Invalid MADT checksum");

        local_apic::set_local_apic_addr(madt_hdr->local_apic_addr);

        lapics = Vector();

        /* We are going to loop through entries after the header until we reach the end */
        uint8_t* ptr = (uint8_t*)(madt_hdr) + sizeof(apic_madt_t);

        while (ptr < (uint8_t*)(madt_hdr) + madt_hdr->hdr.length) { // While there are more headers to be consumed
            apic_hdr_t* header = (apic_hdr_t*) ptr;

            switch(header->type) {
                case LAPIC_TYPE: { // Add the lapic to the list
                    local_apic_t* s = (local_apic_t*)ptr;

                    local_apic_t* lapic = (local_apic_t*) malloc(sizeof(local_apic_t));
                    memcpy((void*) lapic, (void*) s, sizeof(local_apic_t));

                    lapics.add(lapic);

                    num_cores++;
                } break;

                case IOAPIC_TYPE: { // Set the address of the IOAPIC
                    io_apic_t* s = (io_apic_t*) ptr;
                    ioapic::set_addr(s->io_apic_addr);
                } break;

                case IOAPIC_INT_SRC_OVERRIDE: { // These entries tell us how IRQ sources are mapped to global system interrupts
                    interrupt_override_t* int_override = (interrupt_override_t*) ptr;

                    interrupt_override_t* copy = (interrupt_override_t*) malloc(sizeof(interrupt_override_t));
                    memcpy((void*) copy, (void*) int_override, sizeof(interrupt_override_t));

                    int_overrides.add((void*) copy);
                } break;

                default: break;
            }

            ptr += header->length;
        }

        io::print_drv_name("MADT");
        io::printf("Parsed MADT, found %u core(s)\n", num_cores);
    }

    uint32_t madt_remap_irq(uint32_t irq) {
        apic_madt_t* madt_hdr = (apic_madt_t*) get_table("APIC"); // Calling function from acpi.h

        for(int i = 0; i < int_overrides.size(); i++) {
            interrupt_override_t* int_override = (interrupt_override_t*) int_overrides.get(i);

            if(int_override->irq_source == irq) {
                return int_override->glob_sys_interrupt;
            }
        }

        return irq;
    }

    Vector* madt_get_lapics() {
        return &lapics;
    }
}

#include "interrupts.h"

#include <stdio.h>

#include "idt.h"
#include "isr.h"
#include "pit.h"
#include "pic.h"
#include "local_apic.h"
#include "ioapic.h"

#include "../acpi/madt.h"

namespace interrupts {
    
    void init() {
        disable();
        
        idt::set_descriptor(0, (void*) isr::div_by_zero_handler, 0x8E);
        idt::set_descriptor(0x0D, (void*) isr::gp_fault_handler, 0x8E);
        idt::set_descriptor(0xFF, (void*) isr::spurious_interrupt_handler, 0x8E);
        idt::set_descriptor(0x0E, (void*) isr::page_fault_handler, 0x8E);
        idt::set_descriptor(INT_TIMER, (void*) isr::pit_handler, 0x8E);
        idt::set_descriptor(0x01, (void*) isr::debug_handler, 0x8E);
        idt::set_descriptor(0x06, (void*) isr::invalid_opcode_handler, 0x8E);
        idt::load_idtr();

        // Disable the PIC (to use the IO APIC)
        pic::disable();
        local_apic::init();
        ioapic::init();
        pit::init();

        /* Enable IO APIC entries */
        ioapic::set_entry(ioapic::get_addr(), acpi::madt_remap_irq(IRQ_TIMER), INT_TIMER);

        enable(); // Enable interrupts

        io::print_drv_name("Interrupts");
        io::printf("Initialised and enabled interrupt handling and related chips\n");
    }

    void remap_interrupts() {
        // Set up ioapic fully now that all CPUs have been initialised
        ioapic::setup_redtbl();
    }

    void enable() {
        asm("sti");
    }

    void disable() {
        asm("cli");
    }
}

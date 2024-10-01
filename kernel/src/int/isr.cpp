#include "isr.h"

#include <stdio.h>

#include "pit.h"
#include "ioapic.h"

#include <cpu.h>

namespace isr {

    __attribute__((interrupt)) void div_by_zero_handler(isr_frame_t* frame) {
        io::printf("EXCEPTION: DIV BY ZERO handled by CPU %u", cpu::get_lapic_id());

        while(true) {}
    }

    __attribute__((interrupt)) void gp_fault_handler(isr_frame_t* frame) {
        io::printf("EXCEPTION: GENERAL PROTECTION FAULT");

        while(true) {}
    }

    __attribute__((interrupt)) void spurious_interrupt_handler(isr_frame_t* frame) {
        io::printf("EXCEPTION: SPURIOUS INTERRUPT");

        while(true) {}
    }

    __attribute__((interrupt)) void page_fault_handler(isr_frame_t* frame) {
        io::printf("EXCEPTION: PAGE FAULT");

        while(true) {}
    }

    __attribute__((interrupt)) void invalid_opcode_handler(isr_frame_t* frame) {
        io::printf("EXCEPTION: INVALID OPCODE");

        while(true) {}
    }

    __attribute__((interrupt)) void pit_handler(isr_frame_t* frame) {
        pit::tick();
        ioapic::eoi();
    }

    __attribute__((interrupt)) void debug_handler(isr_frame_t* frame) {
        io::printf("Debug %u\n", pit::get_uptime());
    }
}

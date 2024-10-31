#include "smp.h"

#include <vector.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../acpi/madt.h"

#include "../int/local_apic.h"
#include "../int/pit.h"

#include "../mem/memory.h"
#include "../gdt/gdt.h"

#include "../int/interrupts.h"

#include "ap_entry.h"

namespace smp {

    uint8_t running_cpus = 1; // The BSP is already running

    Vector cpus;

    void init() {

        io::print_drv_name("Multiprocessing");
        io::printf("Initialising other CPUs...\n");

        /* First, initialise the cpus vector */

        Vector* lapics = acpi::madt_get_lapics();

        for(int i = 0; i < lapics->size(); i++) {
            acpi::local_apic_t* lapic = (acpi::local_apic_t*) lapics->get(i);
            CPU* cpu = new CPU(lapic->acpi_processor_id, lapic->apic_id, lapic->flags);
            cpus.add(cpu);
        }

        /* Copy the AP trampoline code to the first usable page */
        void* trampoline_addr = memory::upn_to_address(0);
        memory::get_vmm()->map_memory(trampoline_addr, trampoline_addr);
        memcpy((void*) trampoline_addr, (void*) &ap_trampoline, 4096);

        uint32_t bsp_id = local_apic::get_id();

        // Send init to all cpus except self

        for (uint8_t i = 0; i < cpus.size(); i++) {
            CPU* cpu = (CPU*) cpus[i];

            uint8_t apic_id = cpu->get_apic_id();

            // We don't need to try to start the BSP
            if(apic_id == bsp_id) continue;

            uint8_t enabled_bit = (cpu->get_flags() >> 0) & 1;
            uint8_t online_cpbl_bit = (cpu->get_flags() >> 1) & 1;

            if(!enabled_bit && !online_cpbl_bit) {
                io::print_drv_name("SMP");
                io::printf("AP %u is unable to be started\n", apic_id);
                continue;
            }

            // Record how many APs are running before we start this one
            uint8_t aps_running_before = running_cpus;

            /* Set up the parameters for the trampoline code */
            // We pass the parameters directly to space allocated in the asm trampoline code. We add two to skip a jump instruction
            trampoline_params_t* trampoline_params = (trampoline_params_t*)(((uint8_t*) trampoline_addr) + 2);
            trampoline_params->pml4 = (uint32_t) (uint64_t) memory::get_vmm()->get_pml4();
            trampoline_params->started = 0;
            trampoline_params->continue_exec = 0;
            trampoline_params->entry = KERNEL_START + (uint64_t) ap_entry;
            trampoline_params->gdtr = (uint32_t) (uint64_t) (void*) g_gdtr;

            /* Initialise a stack for this CPU */

            void* stack = (void*)((uint64_t) memory::get_allocator()->alloc_page() + 0x1000);
            trampoline_params->stack = (uint32_t)((uint64_t) stack);

            /* Update the APIC ID argument */

            trampoline_params->apic_id = apic_id;

            /* Sending IPIs to start the CPUs 
               
               For both INIT and STARTUP IPIs, the you must first write the target LAPIC ID into bits 24-27 of base + 0x310.

               Then, for INIT:
               - Write 0x4500 to base + 0x300

               For STARTUP:
               - Write 0x4600, or'ed with the page number to start executing at, to base + 0x300
            
            */

           // IPIs are sent as shown at https://wiki.osdev.org/Symmetric_Multiprocessing#Sending_IPIs

            /* Sending an INIT IPI */
            local_apic::out(0x280, 0);
            local_apic::out(0x310, (local_apic::in(0x310) & 0x00FFFFFF) | (cpu->get_apic_id() << 24));
            local_apic::out(0x300, (local_apic::in(0x300) & 0xFFF00000) | 0x004500);

            pit::sleep(10); // 10 milliseconds

            for (int j = 0; j < 2; j++) {

                /* Sending a SIPI */
                local_apic::out(0x280, 0);
                local_apic::out(0x310, (local_apic::in(0x310) & 0x00FFFFFF) | (cpu->get_apic_id() << 24));
                local_apic::out(0x300, (local_apic::in(0x300) & 0xFFF00000) | 0x004600 | (uint32_t) (uint64_t)trampoline_addr);

                if(j == 0) pit::sleep(5); // 5 milliseconds
                else pit::sleep(1000); // 1 second

                // Poll for the 'started' flag
                if(trampoline_params->started == 1) break;
            }

            bool fully_started = false;

            if(trampoline_params->started) {
                
                /* Now we know that THIS AP has started, we can tell it to continue execution of the trampoline code.
                   If another AP is accidentally started, this prevents it from running and messing everything up. */
                trampoline_params->continue_exec = 1;

                /* Adding a timeout, during which the AP should run the trampoline code */
                uint64_t start_time = pit::get_uptime();
                uint64_t timeout_length = 1000;

                while(pit::get_uptime() - start_time < timeout_length) {
                    if(trampoline_params->started && aps_running_before + 1 == running_cpus) {
                        fully_started = true;
                        break;
                    }
                }
            }

            /* Display to the user whether or not the AP has been started */

            if(fully_started) {
                io::print_drv_name("SMP");
                io::printf("Started AP %d\n", apic_id);
                cpu->set_started();
            } else {
                io::print_drv_name("SMP");
                char* reason_str = "Trampoline parameter not set";

                if(trampoline_params->started) {
                    reason_str = "Timeout exceeded";
                }

                io::printf("Failed to start AP %d. Reason: %s\n", apic_id, reason_str);
            }
        }

        /* CLearing up - deleting all of the trampoline code from the first usable page */
        //memset128((void*) trampoline_addr, 0, 4096 / 16); // Doing this causes bad things... sometimes. Why?

        io::print_drv_name("Multiprocessing");
        io::printf("%u CPUs enabled\n", running_cpus);
    }

    CPU::CPU(uint8_t acpi_processor_id, uint8_t apic_id, uint32_t flags) {
        this->m_acpi_processor_id = acpi_processor_id;
        this->m_apic_id = apic_id;
        this->m_flags = flags;

        this->m_started = false;
    }

    CPU::~CPU() {

    }

    uint8_t CPU::get_apic_id() {
        return m_apic_id;
    }

    uint32_t CPU::get_flags() {
        return m_flags;
    }

    void CPU::set_started() {
        m_started = true;
    }
}
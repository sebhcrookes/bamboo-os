#pragma once

#include <stdint.h>

namespace smp {

    extern uint8_t running_cpus;

    struct trampoline_params_t {
        uint32_t pml4;
        uint32_t stack;
        uint32_t gdtr;
        uint8_t started;
        uint8_t continue_exec;
        uint64_t entry;
        uint8_t apic_id;
    } __attribute__((packed));

    class CPU {
        private:
            uint8_t m_acpi_processor_id;
            uint8_t m_apic_id;
            uint32_t m_flags;

            bool m_started;
        public:
            CPU(uint8_t acpi_processor_id, uint8_t apic_id, uint32_t flags);
            ~CPU();

            uint8_t get_apic_id();
            uint32_t get_flags();

            void set_started();
    };

    void init();

    void init_cpu_list();
    void register_cpu(CPU* cpu);

    void start_secondary_cpu(CPU* cpu);

    extern "C" void ap_trampoline();
}

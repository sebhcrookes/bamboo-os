#include "detect.h"

#include <stdio.h>

// CPUID resource: http://www.flounder.com/cpuid_explorer2.htm

namespace cpu {
    char cname[48];
    char* cpu_name;

    static void cpuid(uint32_t reg, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
        __asm__ volatile ("cpuid"
            : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
            : "0" (reg)
        );
    }

    void detect() {
        uint32_t eax, ebx, ecx, edx;

        uint32_t largest_ext_func;
        cpuid(0x80000000, &largest_ext_func, &ebx, &ecx, &edx);

        // Getting the processor brand string
        if(largest_ext_func >= 0x80000004) {
            char cname[48];

            for(int i = 0; i < 3; i++) {
                uint8_t base = i * 16;

                // Each pointer is given 4 characters, so we do cname + base + a multiple of 4, per register
                cpuid(0x80000002 + i, (uint32_t*)(cname + base), (uint32_t*)(cname + base + 4), (uint32_t*)(cname + base + 8), (uint32_t*)(cname + base + 12));
            }

            cpu_name = cname;
            while(cpu_name[0] == ' ') cpu_name++; // Removing padding of ' ' characters

            io::print_drv_name("CPUInfo");
            io::printf("Found CPU with name: '%s'\n", cname);
        }
    }
}
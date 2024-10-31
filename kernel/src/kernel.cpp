#include "kernel.h"

#include <stdio.h>
#include <faults.h>

#include "cpu/detect.h"
#include "gdt/gdt.h"
#include "int/interrupts.h"
#include "mem/memory.h"
#include "mem/allocator.h"
#include "mem/vmm.h"
#include "acpi/acpi.h"
#include "fs/vfs.h"
#include "cpu/smp.h"
#include "pci/pci.h"
#include "pci/pci_drivers.h"

namespace kernel {

    Renderer renderer;
    GDT gdt;

    void init(bamboo_boot_info_t* boot_info) {
        interrupts::disable();

        renderer = Renderer(boot_info);
        renderer.clear(0x1D252C);
        io::init(&renderer);

        print_logo();        

        io::print_drv_name("Renderer");
        io::printf("Renderer initialised\n");

        cpu::detect();

        memory::parse_memory_map(boot_info->mmap);
        memory::allocator_init();
        memory::vmm_init(boot_info);
        memory::kheap_init();

        gdt = GDT();
        gdt.install();

        acpi::init(boot_info->rsdp);

        pci::init();
        pci::initialise_pci_drivers();

        vfs::init();

        interrupts::init();

        smp::init();

        io::print_drv_name("Kernel");
        io::printf("Kernel initialised\n\n");
}

    void print_logo() {
        int c = COL_KERNEL_PRINT;
        renderer.puts(" ____                        _                        ____     _____ \n", c);
        c += 0x1000;
        renderer.puts("|  _ \\                      | |                      / __ \\   / ____|\n", c);
        c += 0x1000;
        renderer.puts("| |_) |   __ _   _ __ ___   | |__     ___     ___   | |  | | | (___  \n", c);
        c += 0x1000;
        renderer.puts("|  _ <   / _` | | '_ ` _ \\  | '_ \\   / _ \\   / _ \\  | |  | |  \\___ \\ \n", c);
        c += 0x1000;
        renderer.puts("| |_) | | (_| | | | | | | | | |_) | | (_) | | (_) | | |__| |  ____) |\n", c);
        c += 0x1000;
        renderer.puts("|____/   \\__,_| |_| |_| |_| |_.__/   \\___/   \\___/   \\____/  |_____/ \n\n", c);
    }
}
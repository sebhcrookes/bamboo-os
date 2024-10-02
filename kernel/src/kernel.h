#pragma once

#include "kentry.h"

#include "renderer.h"
#include "gdt/gdt.h"

namespace kernel {
    void init(bamboo_boot_info_t* boot_info);

    void print_logo();
}
#include "mcfg.h"

#include <faults.h>
#include <vector.h>
#include <stdio.h>

namespace acpi {

    mcfg_hdr_t* mcfg = nullptr;

    Vector entries;
    
    void parse_mcfg() {
        mcfg = (mcfg_hdr_t*) get_table("MCFG");

        faults::assert(is_valid_sdt_checksum(&mcfg->header), "Invalid MCFG checksum");

        io::print_drv_name("MCFG");
        io::printf("Parsed MCFG\n");
    }

    mcfg_hdr_t* get_mcfg() {
        return mcfg;
    }
}
#include "mcfg.h"

#include <panic.h>
#include <vector.h>
#include <stdio.h>

namespace acpi {

    Vector entries;
    
    void parse_mcfg() {
        sdt_hdr_t* mcfg_hdr = get_table("MCFG");
        faults::assert(is_valid_sdt_checksum(mcfg_hdr), "Invalid MCFG checksum");

        entries = Vector();

        uint64_t num_entries = (mcfg_hdr->length - sizeof(sdt_hdr_t)) / sizeof(config_space_base_addr_t);

        io::printf("%u\n", num_entries);
    }

}
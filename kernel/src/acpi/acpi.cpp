#include "acpi.h"

#include <faults.h>
#include <stdio.h>

#include "madt.h"
#include "mcfg.h"

namespace acpi {
    rsdp_v2_t* rsdp;
    sdt_hdr_t* xsdt;

    void init(void* rsdp_address) {
        parse_rsdp(rsdp_address);
        parse_xsdt();

        io::print_drv_name("ACPI");
        io::printf("Validated RSDP and XSDT. Found:\n");

        parse_xsdt_tables();
    }

    void parse_rsdp(void* rsdp_address) {
        /* === Loading the RSDP V1 structure === */

        rsdp_t* table_v1 = (rsdp_t*) rsdp_address;
        
        /* === Validating RSDP signature === */
        // Checking that signature == "RSD PTR "

        bool valid_signature = true;
        const char* expected_sig = "RSD PTR ";

        for(int i = 0; i < 8; i++) {
            if(table_v1->signature[i] != expected_sig[i]) {
                valid_signature = false;
                break;
            }
        }

        faults::assert(valid_signature, "Invalid RSDP V1 signature");

        /* === Validating RSDP V1.0 checksum === */
        /* To validate, add up the values of every byte in the structure, then
           cast to a uint8_t. If the value == 0, the structure is valid */
        
        uint64_t sum = 0;
        for(int i = 0; i < sizeof(rsdp_t); i++) {
            sum += ((uint8_t*) table_v1)[i];
        }

        faults::assert(sum % 0x100 == 0, "Invalid RSDP V1 checksum");

        /* === Checking revision === */
        /* Currently, BambooOS only supports V2.0 ACPI and onwards.
           The revision will equal 1 if V1, and equal 2 for anything later */

        faults::assert(table_v1->revision == 2, "Invalid ACPI version - only V2.0 and later supported");

        /* === Loading the full RSDP V2 structure === */

        rsdp_v2_t* table_v2 = (rsdp_v2_t*) rsdp_address;

        /* === Validating RSDP V2 checksum === */
        // To validate, do the same as with V1, with the whole table

        uint64_t sum2 = 0;
        for(int i = 0; i < sizeof(rsdp_v2_t); i++) {
            sum2 += ((uint8_t*) table_v2)[i];
        }

        faults::assert(sum2 % 0x100 == 0, "Invalid RSDP V2 checksum");

        // Everything is in order and the table is valid
        rsdp = table_v2;
    }
    
    void parse_xsdt() {
        sdt_hdr_t* xsdt_hdr = (sdt_hdr_t*) rsdp->xsdt_address;

        bool signature_valid = is_signature_equal(xsdt_hdr, "XSDT");
        faults::assert(signature_valid, "Invalid XSDT header signature");

        bool valid = is_valid_sdt_checksum(xsdt_hdr);
        faults::assert(valid, "Invalid XSDT header checksum");

        xsdt = xsdt_hdr;
    }

    void parse_xsdt_tables() {
        uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

        // After the XSDT header, there is an array of pointers to other SDTs
        uint64_t* sdts = (uint64_t*)(xsdt + 1);

        for(int i = 0; i < entries; i++) {
            io::printf(" - ");
            sdt_hdr_t* header = (sdt_hdr_t*)sdts[i];
            for(int j = 0; j < 4; j++) {
                io::printf("%c", header->signature[j]);
            }

            /* Check if table should be parsed */

            bool has_parsed = true;

            if(is_signature_equal(header, "APIC")) {
                io::printf(": ");
                parse_madt();
            } else if(is_signature_equal(header, "MCFG")) {
                io::printf(": ");
                parse_mcfg();
            } else {
                has_parsed = false;
                io::printf("\n");
            }
        }
    }

    sdt_hdr_t* get_table(const char* signature) {
        uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

        // After the XSDT header, there is an array of pointers to other SDTs
        uint64_t* sdts = (uint64_t*)(xsdt + 1);

        for(int i = 0; i < entries; i++) {
            sdt_hdr_t* hdr = (sdt_hdr_t*)(sdts[i]);

            if(is_signature_equal(hdr, signature)) return hdr;
        }

        return nullptr;
    }

    /* Helper functions */

    bool table_exists(const char* signature) {
        uint64_t entries = (xsdt->length - sizeof(sdt_hdr_t)) / 8;

        // After the XSDT header, there is an array of pointers to other SDTs
        uint64_t* sdts = (uint64_t*)(xsdt + 1);

        for(int i = 0; i < entries; i++) {
            sdt_hdr_t* hdr = (sdt_hdr_t*)(sdts[i]);

            if(is_signature_equal(hdr, signature)) return true;
        }

        return false;
    }

    bool is_signature_equal(sdt_hdr_t* hdr, const char* signature) {
        for(int i = 0; i < 4; i++) {
            if(hdr->signature[i] != signature[i]) return false;
        }

        return true;
    }

    bool is_valid_sdt_checksum(sdt_hdr_t* hdr) {
        /* This function sums all of the bytes in the header (including the checksum)
           If the result (modulo 0x100, aka. cast to a uint8) is 0, then the hdr is valid */
        uint64_t sum = 0;
        for(uint64_t i = 0; i < hdr->length; i++) {
            sum += ((uint8_t*) hdr)[i];
        }
        return (sum % 0x100 == 0);
    }
}
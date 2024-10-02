#pragma once

#include <stdint.h>

namespace acpi {

    /* Structs */

    struct rsdp_t {
        char signature[8];
        uint8_t checksum;
        char OEMID[6];
        uint8_t revision;
        uint32_t rsdt_address; // The PHYSICAL address to the RSDT
    } __attribute__ ((packed));

    struct rsdp_v2_t {
        rsdp_t rsdt;

        uint32_t length;
        uint64_t xsdt_address; // The PHYSICAL address to the XSDT
        uint8_t extended_checksum;
        uint8_t reserved[3];
    } __attribute__((packed));

    struct sdt_hdr_t {
        char signature[4];
        uint32_t length;
        uint8_t revision;
        uint8_t checksum;
        char OEMID[6];
        char OEM_table_id[8];
        uint32_t OEM_revision;
        uint32_t creator_id;
        uint32_t creator_revision;
    } __attribute__((packed));

    /* Functions */

    void init(void* rsdp_address);

    void parse_rsdp(void* rsdp_address);
    void parse_xsdt();

    void parse_xsdt_tables();

    sdt_hdr_t* get_table(const char* signature);

    /* Helper functions */
    bool table_exists(const char* signature);
    bool is_signature_equal(sdt_hdr_t* hdr, const char* signature);
    bool is_valid_sdt_checksum(sdt_hdr_t* hdr);
}
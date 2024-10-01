#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Structs */

typedef struct {
    uint32_t d0;
    uint16_t d1;
    uint16_t d2;
    uint64_t d3;
} befi_guid_t;

// struct befi_system_table_t {
//     efi_table_header_t header;

//     wchar_t* firmware_vendor;
//     uint32_t firmware_revision;

//     efi_handle_t console_in_handle;
//     simple_input_interface_t* console_in;

//     efi_handle_t console_out_handle;
//     simple_text_output_interface_t* console_out;

//     efi_handle_t console_error_handle;
//     simple_text_output_interface_t* console_error;

//     efi_runtime_services_t* runtime_services;
//     efi_boot_services_t* boot_services;

//     uintn_t config_table_entries;
//     efi_configuration_table_t* config_table;
// };

/* GUIDs */

#define ACPI_V1_TABLE_GUID (befi_guid_t) { 0xeb9d2d30, 0x2d88, 0x11d3, 0x0090273fc14d }
#define ACPI_V2_TABLE_GUID (befi_guid_t) { 0x8868e871, 0xe4f1, 0x11d3, 0x81883cc78000 }

/* Functions */

bool guid_equal(befi_guid_t f, befi_guid_t s) {
	if(f.d0 == s.d0
	&& f.d1 == s.d1
	&& f.d2 == s.d2
	&& f.d3 == s.d3) {
		return true;
	}

	return false;
}

/* Global variables */
// extern befi_system_table_t* ST;
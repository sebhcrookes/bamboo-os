#include "efi_memory.h"

#define NUM_STRINGS 16
const char* efi_mem_strings[] = {
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServicesCode",
    "EfiRuntimeServicesData",
    "EfiConventionalMemory",
    "EfiUnusableMemory",
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS",
    "EfiMemoryMappedIO",
    "EfiMemoryMappedIOPortSpace",
    "EfiPalCode",
    "EfiPersistentMemory",
    "EfiMaxMemoryType"
};

const char* efi_mem_type_as_str(uint32_t type) {
    if(type >= NUM_STRINGS) return "";
    return efi_mem_strings[type];
}
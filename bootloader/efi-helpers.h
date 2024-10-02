#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Functions */

bool guid_equal(efi_guid_t f, efi_guid_t s) {
	if(f.Data1 == s.Data1
	&& f.Data2 == s.Data2
	&& f.Data3 == s.Data3
	&& *(uint64_t*) &f.Data4[0] == *(uint64_t*) &s.Data4[0]) {
		return true;
	}

	return false;
}
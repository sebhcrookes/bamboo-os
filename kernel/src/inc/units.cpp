#include "units.h"

uint64_t b_to_mb(uint64_t bytes) {
    return bytes / 1048576;
}

uint64_t b_to_gb(uint64_t bytes) {
    return bytes / 1073741824;
}
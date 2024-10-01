#include "ap_entry.h"

#include "smp.h"

#include <stdio.h>

extern "C" void ap_entry(uint8_t cpu_id) {
    smp::running_cpus++;
    while(true) {}
}

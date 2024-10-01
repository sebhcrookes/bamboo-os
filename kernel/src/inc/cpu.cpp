#include "cpu.h"

#include "../int/local_apic.h"

namespace cpu {
    int get_lapic_id() {
        return local_apic::get_id();
    }
}

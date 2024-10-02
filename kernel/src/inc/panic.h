#pragma once

namespace faults {
    void assert(bool condition, const char* reason);
    void panic(const char* reason);
}
#pragma once

#include <stdint.h>
#include <vector.h>

namespace storage {
    struct stor_drv_t { 
        uint32_t uuid;

        bool (*read)(uint32_t, uint64_t, uint32_t, char*);
        bool (*write)(uint32_t, uint64_t, uint32_t, char*);
    };

    void init();

    /* Each driver should have two functions:

        read(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer);

        write(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer);

    */

    uint32_t add_driver(bool (*read)(uint32_t, uint64_t, uint32_t, char*), bool (*write)(uint32_t, uint64_t, uint32_t, char*));

    bool read(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer);
    bool write(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer);

    uint32_t allocate_uuid();

    Vector* get_drivers();
}
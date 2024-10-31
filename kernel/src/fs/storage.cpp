#include "storage.h"

#include <stdio.h>
#include <stdlib.h>

namespace storage {

    Vector* drivers;

    void init() {
        drivers = new Vector();
    }

    uint32_t add_driver(bool (*read)(uint32_t, uint64_t, uint32_t, char*), bool (*write)(uint32_t, uint64_t, uint32_t, char*)) {
        stor_drv_t* drv = (stor_drv_t*) malloc(sizeof(stor_drv_t));

        drv->read = read;
        drv->write = write;
        drv->uuid = allocate_uuid();

        // Here we find where the new driver should be inserted so the list stays sorted (based on UUID)

        if(drivers->size() == 0) {
            drivers->add((void*) drv);
            return drv->uuid;
        }

        uint32_t last = 0;

        if(drv->uuid == 0) {
            drivers->insert(0, (void*) drv);
            return drv->uuid;
        }

        for(int i = 1; i < drivers->size(); i++) {
            stor_drv_t* d = (stor_drv_t*) drivers->get(i);
            if(last < drv->uuid && d->uuid > drv->uuid) {
                drivers->insert(i, (void*) drv);
                return drv->uuid;
            }
        }

        drivers->add((void*) drv);
        return drv->uuid;
    }

    bool read(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer) {
        for(int i = 0; i < drivers->size(); i++) {
            stor_drv_t* drv = (stor_drv_t*) drivers->get(i);

            if(drv->uuid == uuid) {
                return drv->read(uuid, sector, sector_count, buffer);
            }
        }
        return false;
    }

    bool write(uint32_t uuid, uint64_t sector, uint32_t sector_count, char* buffer) {
        for(int i = 0; i < drivers->size(); i++) {
            stor_drv_t* drv = (stor_drv_t*) drivers->get(i);

            if(drv->uuid == uuid) {
                return drv->write(uuid, sector, sector_count, buffer);
            }
        }
        return false;
    }

    uint32_t allocate_uuid() {
        if(drivers->size() == 0) return 0;

        // This code goes through the list and finds any gaps to allocate a new UUID in
        // If there are no gaps, we return a new UUID at the end

        uint32_t last = 0;

        if(((stor_drv_t*) drivers->get(0))->uuid == 1) {
            return 0;
        }

        for(int i = 0; i < drivers->size(); i++) {
            stor_drv_t* current = (stor_drv_t*) drivers->get(1);
            if(last + 1 != current->uuid) {
                return last + 1;
            }
        }

        return drivers->size();
    }

    Vector* get_drivers() {
        return drivers;
    }
}
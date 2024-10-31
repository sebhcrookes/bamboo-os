#pragma once

#include <stdint.h>

class Vector {
    private:
        uint64_t m_length;
        void** m_items;
    public:
        Vector();
        ~Vector();

        /* Returns the size of the vector */
        uint64_t size();

        /* Adds an item to the vector at the end */
        void add(void* address);
        
        /* Gets an item at the given index
        Returns nullptr if the index was out of bounds */
        void* get(uint64_t index);

        /* Inserts an item at the given index */
        void insert(uint64_t index, void* address);

        /* Frees all items in the vector
        Warning: presumes all items are actually freeable */
        void free_all();

        /* Deletes all items in the vector without freeing them */
        void delete_all();

        /* Deletes the item at the given index and shifts everything
        ahead of that index down one item */
        void delete_at(uint64_t index);

        void* operator[] (uint64_t i) const { return m_items[i]; }
        void*& operator[] (uint64_t i) { return m_items[i]; }

};
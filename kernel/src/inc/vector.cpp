#include "vector.h"

#include <stdlib.h>
#include <memory.h>

/*
    The Vector class allows for arrays which can grow dynamically in BambooOS.
    A vector is simply a void** which changes size dynamically depending on 
    whether you add / delete m_items from it.
*/

Vector::Vector() {
    this->m_length = 0;
    this->m_items = (void**) malloc(sizeof(void*));
}

Vector::~Vector() {
    m_length = 0;
    free(m_items);
}

uint64_t Vector::size() {
    return this->m_length;
}

void Vector::add(void* address) {
    m_length++;
    m_items = (void**) realloc((void*) m_items, m_length * sizeof(void*));
    m_items[m_length - 1] = address;
}

void* Vector::get(uint64_t index) {
    if(index >= m_length)
        return nullptr;

    return m_items[index];
}

void Vector::insert(uint64_t index, void* address) {
    if(index > m_length) return;
    m_length++;
    m_items = (void**) realloc((void*) m_items, m_length * sizeof(void*));
    for (int i = m_length - 1; i > index; i--) {
        m_items[i] = m_items[i - 1];
    }
    
    m_items[index] = address;
}

void Vector::free_all() {
    for(int i = 0; i < m_length; i++) {
        free(m_items[i]);
    }
    delete_all();
}

void Vector::delete_all() {
    m_length = 0;
    free(m_items);
    m_items = (void**) malloc(sizeof(void*));
}

void Vector::delete_at(uint64_t index) {
    if(index + 1 < m_length) {
        for(int i = index + 1; i < m_length; i++) {
            m_items[i - 1] = m_items[i];
        }
    }
    m_length--;
    //m_items = (void**) mem::heap::realloc((void*) m_items, m_length * sizeof(void*));
}
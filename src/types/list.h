#pragma once
#include <stddef.h>

#include "success.h"

/*
Takes in a "T" (eg. char)
Takes in a "size" variable (where the current size is stored)
Takes in a "capacity" variable (where the current capacity is stored)
Takes in a "buffer" variable
Takes in a boolean for `clear_buffer`. If true, safely clear the buffer
  after each resize
Note that the `clear_buffer` depends on the compiler optimisations so might
  not do anything

Example usage:
    LIST_BEGIN(char, size, buf_size, buf, false)
    while (true) {
        char ch = getchar();
        if (ch == EOF) { break; }
        buf[size++] = ch;
        LIST_APPENDED(char, size, buf_size, buf, false)
    }
    buf[size++] = '\x00;

Note that in the example, after the `buf[size++] = '\x00` there should be
  a LIST_APPENDED but since we won't be adding anything else to buf, it's
  safe
*/

#define LIST_BEGIN(T, size, capacity, buffer, clear_buffer) \
    size_t size = 0; \
    size_t capacity = 10; \
    T* buffer = (T*) malloc(capacity*sizeof(T));

// Same arguments as LIST_BEGIN
#define LIST_APPENDED(T, size, capacity, buffer, clear_buffer) { \
        if (size+1 == capacity) { \
            size_t old_capacity = capacity; \
            capacity = capacity + (capacity>>1); \
            T* new_buffer = (T*) malloc(capacity*sizeof(T)); \
            for (size_t _i=0; _i<size; _i++) { new_buffer[_i] = buffer[_i]; } \
            if (clear_buffer) { \
                list_safe_clear_mem((void*) buffer, old_capacity*sizeof(T)); \
            } \
            free(buffer); \
            buffer = new_buffer; \
        } \
    };

// Size is in bytes (look at comment above list.c@list_safe_clear_mem)
Success list_safe_clear_mem(volatile void* mem, size_t size);

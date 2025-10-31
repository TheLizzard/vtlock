#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// An immutable Bytes struct
// Always check if bytes.data is NULL, as an error may have occured
// If bytes.len == 0, bytes.data will always be NULL
typedef struct {
    const size_t len;
    const uint8_t* restrict data;
} Bytes;

// If not NULL terminated, undefined behaviour
Bytes bytes_from_charp(const char* restrict data);
// Create a Bytes object from data buffer (stack or heap)
Bytes bytes_from_data(const uint8_t* restrict data, size_t maxlen);
/*
Constructs a Bytes object from data assumed to be on the heap.
The bytes object takes control of the `data` pointer. Don't free it
*/
Bytes bytes_from_heap_data(const uint8_t* data, size_t size);
// Create an empty Bytes object
Bytes bytes_empty();
// Check if 2 Byte objects contain the same data
bool bytes_eq(const Bytes* restrict first, const Bytes* restrict second);
// Deepcopy a bytes object (only point is to be able to free original)
Bytes bytes_copy(const Bytes* restrict other);
// Free a bytes object. After this, the the object's data will be NULL
void bytes_free(Bytes* restrict bytes);
// Concatonate 2 Bytes objects
Bytes bytes_concat(const Bytes* restrict first, const Bytes* restrict second);
// Print the data in a bytes object in hex
void bytes_print(const Bytes* restrict data);
void bytes_println(const Bytes* restrict data);
/*
Read `size` data from a file.
On failure, prints error msg and returns partial result.
Caller must check if `output.len == size`
*/
Bytes bytes_file_read(FILE* file, size_t size, bool print_err);

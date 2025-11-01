#include <stdlib.h>
#include <stdio.h>

#include "bytes.h"


Bytes bytes_empty() { return (Bytes) {0, NULL}; }

Bytes bytes_from_charp(const char* restrict data) {
    if (data == NULL) { return bytes_empty(); }
    size_t len = 0;
    while (data[len] != '\x00') { len++; }
    len++; // For the NULL byte
    uint8_t* new_buf = (uint8_t*) malloc(len*sizeof(uint8_t));
    if (new_buf == NULL) { return bytes_empty(); }
    for (size_t i=0; i<len; i++) {
        new_buf[i] = (uint8_t) data[i];
    }
    return bytes_from_heap_data(new_buf, len);
}

Bytes bytes_from_data(const uint8_t* restrict data, size_t len) {
    if ((data == NULL) || (len == 0)) { return bytes_empty(); }
    uint8_t* new_buf = (uint8_t*) malloc(len*sizeof(uint8_t));
    if (new_buf == NULL) { return bytes_empty(); }
    for (size_t i=0; i<len; i++) { new_buf[i] = data[i]; }
    return bytes_from_heap_data(new_buf, len);
}

Bytes bytes_from_heap_data(const uint8_t* data, size_t size) {
    if (size == 0) {
        if (data != NULL) { free((void*) data); }
        return bytes_empty();
    }
    return (Bytes) {size, data};
}

void bytes_free(Bytes* restrict bytes) {
    if (bytes->data == NULL) { return; }
    free((void*) bytes->data);
    bytes->data = NULL;
}

bool bytes_eq(const Bytes* restrict first, const Bytes* restrict second) {
    if ((first->data == NULL) || (second->data == NULL)) {
        if ((first->data == NULL) && (second->data == NULL)) {
            // bytes_empty() is a valid object
            return (first->len == 0) && (second->len == 0);
        }
        return false;
    }
    if (first->len != second->len) { return false; }
    for (size_t i=0; i<first->len; i++) {
        if (first->data[i] != second->data[i]) { return false; }
    }
    return true;
}

Bytes bytes_concat(const Bytes* restrict first, const Bytes* restrict second) {
    if (first->data == NULL) {
        return bytes_copy(second);
    } else if (second->data == NULL) {
        return bytes_copy(first);
    } else {
        size_t len = first->len + second->len;
        uint8_t* data = (uint8_t*) malloc(len*sizeof(uint8_t));
        if (data == NULL) { return bytes_empty(); }
        for (size_t i=0; i<first->len; i++) {
            data[i] = first->data[i];
        }
        for (size_t i=0; i<second->len; i++) {
            data[i+first->len] = second->data[i];
        }
        return bytes_from_heap_data(data, len);
    }
}

Bytes bytes_copy(const Bytes* restrict other) {
    if (other->data == NULL) { return bytes_empty(); }
    uint8_t* data = (uint8_t*) malloc(other->len*sizeof(uint8_t));
    if (data == NULL) { return bytes_empty(); }
    for (size_t i=0; i<other->len; i++) { data[i] = other->data[i]; }
    return bytes_from_heap_data(data, other->len);
}

void bytes_print(const Bytes* restrict data) {
    if (data->data == NULL) {
        printf("<empty/freed bytes object>");
    } else {
        for (size_t i=0; i<data->len; i++) {
            printf("%02x", data->data[i]);
        }
    }
}

void bytes_println(const Bytes* restrict data) {
    bytes_print(data);
    puts("");
}

const char* FILE_READ_FAIL_MSG = "fread failed. It only read %zu out of %zu " \
                                 "bytes requested\n";
Bytes bytes_file_read(FILE* file, size_t size, bool print_err) {
    if (file == NULL) { return bytes_empty(); }
    uint8_t* data = (uint8_t*) malloc(size);
    if (data == NULL) { return bytes_empty(); }
    size_t read_size = fread(data, 1, size, file);
    if (print_err && (read_size != size)) {
        fprintf(stderr, FILE_READ_FAIL_MSG, read_size, size);
    }
    return bytes_from_heap_data(data, read_size);
}

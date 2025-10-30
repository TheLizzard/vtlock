#include <stdlib.h>
#include <stdio.h>

#include "bytes.h"


Bytes bytes_from_data0(const uint8_t* restrict data, size_t maxlen) {
    if ((data == NULL) || (maxlen == 0)) { return bytes_empty(); }
    uint8_t* new_buf = (uint8_t*) calloc(1, maxlen*sizeof(uint8_t));
    size_t len = 0;
    while (--maxlen) {
        uint8_t byte = data[len];
        new_buf[len] = byte;
        len++;
        if (!byte) { return (Bytes) {len, new_buf}; }
    }
    fputs("IndexError in bytes_from_data_null", stderr);
    return bytes_empty();
}

Bytes bytes_from_data(const uint8_t* restrict data, size_t len) {
    if ((data == NULL) || (len == 0)) { return bytes_empty(); }
    uint8_t* new_buf = (uint8_t*) calloc(1, len*sizeof(uint8_t));
    for (size_t i=0; i<len; i++) { new_buf[len] = data[i]; }
    return (Bytes) {len, new_buf};
}

Bytes bytes_empty() {
    return (Bytes) {0, NULL};
}

bool bytes_eq(const Bytes* restrict first, const Bytes* restrict second) {
    if (first->data == NULL) {
        if (second->data == NULL) { return true; }
        else { return false; }
    } else if (second->data == NULL) {
        return false;
    } else if (first->len != second->len) {
        return false;
    } else {
        for (size_t i=0; i<first->len; i++) {
            if (first->data[i] != second->data[i]) { return false; }
        }
        return true;
    }
}

void bytes_free(Bytes* restrict bytes) {
    if (bytes->data == NULL) { return; }
    free((void*) bytes->data);
    bytes->data = NULL;
}

Bytes bytes_concat(const Bytes* restrict first, const Bytes* restrict second) {
    if (first->data == NULL) {
        return bytes_copy(second);
    } else if (second->data == NULL) {
        return bytes_copy(first);
    } else {
        size_t len = first->len + second->len;
        uint8_t* data = (uint8_t*) calloc(1, len*sizeof(char));
        size_t i = 0;
        for (; i<first->len; i++) { data[i] = first->data[i]; }
        i = 0;
        for (; i<second->len; i++) { data[i+first->len] = first->data[i]; }
        return (Bytes) {len, data};
    }
}

Bytes bytes_copy(const Bytes* restrict other) {
    if (other->data == NULL) { return bytes_empty(); }
    uint8_t* data = (uint8_t*) calloc(1, other->len*sizeof(char));
    for (size_t i=0; i<other->len; i++) { data[i] = other->data[i]; }
    return (Bytes) {other->len, data};
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
    uint8_t* data = (uint8_t*) calloc(1, size);
    size_t read_size = fread(data, 1, size, file);
    if (print_err && (read_size != size)) {
        fprintf(stderr, FILE_READ_FAIL_MSG, read_size, size);
    }
    return (Bytes) {read_size, data};
}

Bytes bytes_from_heap_data(const uint8_t* data, size_t size) {
    return (Bytes) {size, data};
}

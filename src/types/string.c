#include <stdlib.h>
#include <stdio.h>

#include "string.h"


uint8_t count_first_zero(uint8_t number) {
    /* Counts the number of leading set bits before the first zero bit */
    for (uint8_t i=0; i<8; i++) {
        if ((number & 1<<(7-i)) == 0) { return i; }
    }
    return 8;
}


size_t utf8_next(const Bytes* bytes, size_t idx) {
    if (idx >= bytes->len) { return INVALID_IDX; }
    if ((bytes->data[idx] & 0b10000000) == 0) { // Top bit is 0
        return idx + 1; // ASCII character
    }
    // Continuation byte with no leading byte
    if ((bytes->data[idx] & 0b11000000) != 0b11000000) {
        return INVALID_IDX;
    }
    // Read continuation bytes
    uint8_t continuation_bytes = count_first_zero(bytes->data[idx]) - 1;
    idx++; // Read over the leading byte
    // Make sure that there are enough bytes in the buffer
    if (idx+continuation_bytes > bytes->len) { return INVALID_IDX; }
    // Check next `continuation_bytes` bytes are continuation bytes
    for (size_t j=0; j<continuation_bytes; j++) {
        // Check invalid continuation byte
        if ((bytes->data[idx+j] & 0b11000000) != 0b10000000) {
            return INVALID_IDX;
        }
    }
    return idx + continuation_bytes;
}

String string_from_bytes(Bytes bytes, Encoding encoding) {
    if ((bytes.len == 0) || (bytes.data == NULL)) {
        return (String) {encoding, INVALID_SIZE, bytes};
    }
    if (bytes.data[bytes.len-1] != '\x00') { /* Bytes must be NULL terminated */
        return (String) {encoding, INVALID_SIZE, bytes};
    }
    size_t len = 0;
    switch (encoding) {
        case ENCODING_ASCII:
            len = bytes.len - 1; /* Account for the NULL */
            break;
        case ENCODING_UTF8:
            size_t idx = 0;
            while (idx < bytes.len) {
                idx = utf8_next(&bytes, idx);
                if (idx == INVALID_IDX) { len = INVALID_SIZE; break; }
                len++;
            }
            if (len != INVALID_SIZE) { len--; } /* Account for the NULL */
            break;
        case ENCODING_UTF32:
            size_t data_len = bytes.len - 4; /* Account for the 4 byte NULL */
            len = (data_len&3) ? INVALID_SIZE : (data_len >> 2);
            break;
        default: /* Unknown encoding */
            len = INVALID_SIZE;
            break;
    }
    return (String) {encoding, len, bytes};
}

void string_free(String* str) {
    bytes_free((Bytes*) &str->bytes);
}

String string_concat(const String* str1, const String* str2) {
    if ((str1->encoding != str2->encoding) || \
        (str1->len == INVALID_SIZE) || \
        (str2->len == INVALID_SIZE) || \
        (str1->bytes.data == NULL) || \
        (str2->bytes.data == NULL)
       ) { return (String) {ENCODING_ASCII, INVALID_SIZE, bytes_empty()}; }
    switch (str1->encoding) {
        case ENCODING_ASCII:
        case ENCODING_UTF8:
        case ENCODING_UTF32:
            uint8_t null_byte_size = (str1->encoding==ENCODING_UTF32) ? 4 : 1;
            size_t new_buf_size = str1->bytes.len + str1->bytes.len - \
                                  null_byte_size; /* Account for NULL byte/s */
            uint8_t* new_buf = (uint8_t*) malloc(new_buf_size);
            if (new_buf == NULL) { break; }
            // Copy first string without trailing NULL
            for (size_t i=0; i<str1->bytes.len-null_byte_size; i++) {
                new_buf[i] = str1->bytes.data[i];
            }
            // Copy second string without trailing NULL
            for (size_t i=0; i<str2->bytes.len-null_byte_size; i++) {
                new_buf[i+str1->bytes.len-null_byte_size] = str2->bytes.data[i];
            }
            // Set NULL byte
            for (size_t i=0; i<null_byte_size; i++) {
                new_buf[new_buf_size-null_byte_size+i] = '\x00';
            }
            Bytes bytes = bytes_from_heap_data(new_buf, new_buf_size);
            return (String) {str1->encoding, str1->len+str2->len, bytes};
        default:
            break;
    }
    // Encoding not implemented or another error
    return (String) {ENCODING_ASCII, INVALID_SIZE, bytes_empty()};
}

size_t string_next_char_start(const String* string, size_t idx) {
    if (string->bytes.data == NULL) { return INVALID_IDX; }
    switch (string->encoding) {
        case ENCODING_ASCII:
            // -1 to account for NULL byte
            if (idx >= string->bytes.len-1) { return INVALID_IDX; }
            return idx + 1;
        case ENCODING_UTF8:
            return utf8_next(&string->bytes, idx);
        case ENCODING_UTF32:
            // Check idx is a multiple of 4
            if (idx & 3) { return INVALID_IDX; }
            // -4 to account for NULL bytes
            if (idx >= 4*string->bytes.len-4) { return INVALID_IDX; }
            return idx + 4;
        default: /* Unknown encoding */
            return INVALID_IDX;
    }
}

String string_from_charp(const char* data, const Encoding encoding) {
    return string_from_bytes(bytes_from_charp(data), encoding);
}

bool string_startswith(const String* string, const String* prefix) {
    if (string->encoding != prefix->encoding) { return false; }
    if (string->len == INVALID_SIZE) { return false; }
    if (prefix->len == INVALID_SIZE) { return false; }
    if (string->bytes.data == NULL) { return false; }
    if (prefix->bytes.data == NULL) { return false; }
    if (string->len < prefix->len) { return false; }
    for (size_t i=0; i<prefix->bytes.len; i++) {
        if (string->bytes.data[i] != prefix->bytes.data[i]) { return false; }
    }
    return true;
}

// Same as python's string's string[start:end]
// INVALID_SIZE is used to encode missing argument
// no -ve values, end must be >= start
String string_substring(const String* string, size_t start, size_t end) {
    if (string->bytes.data == NULL) {
        return (String) {ENCODING_ASCII, INVALID_SIZE, bytes_empty()};
    }
    if (string->len == INVALID_SIZE) {
        return (String) {ENCODING_ASCII, INVALID_SIZE, bytes_empty()};
    }
    if (start == INVALID_SIZE) { start = 0; }
    if (end > string->len) { end = string->len; }
    size_t start_idx = 0;
    for (size_t i=0; i<start; i++) {
        start_idx = string_next_char_start(string, start_idx);
    }
    size_t size = end - start;
    size_t end_idx = start_idx;
    for (size_t i=0; i<size; i++) {
        end_idx = string_next_char_start(string, end_idx);
    }
    uint8_t null_byte_size = (string->encoding==ENCODING_UTF32) ? 4 : 1;
    size_t buf_size = size + null_byte_size;
    uint8_t* new = (uint8_t*) malloc(buf_size);
    for (size_t i=start_idx; i<end_idx; i++) {
        new[i-start_idx] = string->bytes.data[i];
    }
    for (size_t i=end_idx; i<end_idx+null_byte_size; i++) { new[i] = '\x00'; }
    return string_from_bytes(bytes_from_heap_data(new, buf_size),
                             string->encoding);
}


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////// TESTING ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/*
const char* UTF8_TESTS[] = {
    "", "X",
    "#", "a", "~", "£", "░",  "▒", "𒀃", "X",
    "##", "#a", "aa", "££", "a£", "£a", "██", "£█", "█£", "█a", "█a", "X",
    "𒀃𒀃𒀃", "aaa", "a𒀃a", "𒀃𒀃a", "a𒀃𒀃", "£𒀃£", "𒀃𒀃£", "𒀃𒀃█", "█𒀃𒀃", "𒀃█𒀃", "X",
    NULL,
};

int main() {
    for (size_t i=0; UTF8_TESTS[i]!=NULL; i++) {
        if (UTF8_TESTS[i][0] == 'X') {
            if (UTF8_TESTS[i][1] == '\x00') {
                puts(""); continue;
            }
        }
        Bytes bytes = bytes_from_charp(UTF8_TESTS[i]);
        String string = bytes_decode(bytes, ENCODING_UTF8);
        printf("%lu ", string.len);
    }
    return 0;
}


const char* data = "String: █, 𒀃!";
int main() {
    Bytes bytes = bytes_from_charp(data);
    String string = bytes_decode(bytes, ENCODING_UTF8);
    ITERATE_OVER_STRING(&string, character, character_len, idx_start, idx_end, {
        printf("idx_start=%lu idx_end=%lu size=%i chr='%s'\n",
               idx_start, idx_end, character_len, character);
    })
    return 0;
}
// */

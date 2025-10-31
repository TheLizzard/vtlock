#pragma once
#include "bytes.h"


typedef enum {
    ENCODING_ASCII,
    ENCODING_UTF8,
    ENCODING_UTF32,
} Encoding;


typedef struct {
    const Encoding encoding;
    const size_t len; /* Without counting the NULL */
    const Bytes bytes; /* Always NULL terminated */
} String;

#define INVALID_SIZE ((size_t) -1)
#define INVALID_IDX INVALID_SIZE


/*
Caller must check and handle `string.len == INVALID_SIZE`
We take control over the bytes. Don't free it
*/
String string_from_bytes(Bytes bytes, Encoding encoding);
// The string object is no longer valid
void string_free(String* str);
// Assumes NULL terminated
String string_from_charp(const char* data, const Encoding encoding);
String string_concat(const String* str1, const String* str2);
size_t string_next_char_start(const String* string, size_t idx);

// Untested
bool string_startswith(const String* string, const String* preffix);
// Untested
String string_substring(const String* string, size_t start, size_t end);


/*
Takes in a String*, <var name> * 4, <code>
And runs the code in <code> for every character (including terminating
   NULL) in the String*.
At each iteration, it sets the 1st <var name> to a `const char*` of
  the current character (NULL terminated). The size of the character
  in bytes is stored in the 2nd <var name>.
The 3rd and 4th <var name> are used to store the `start` and `end`
  of the current character (both are in bytes not characters)
*/
#define ITERATE_OVER_STRING(string_ptr, chr, chr_size, start, end, code) { \
    const String* _tmp_string_ptr = (const String*) (string_ptr); \
    size_t start = 0; \
    while (start != INVALID_IDX) { \
        size_t end = string_next_char_start(_tmp_string_ptr, start); \
        uint8_t chr_size = (uint8_t) (end - start); \
        uint8_t chr[8]; \
        for (size_t i=0; i<chr_size; i++) { \
            chr[i] = _tmp_string_ptr->bytes.data[start+i]; \
        } \
        for (size_t i=chr_size; i<8; i++) { chr[i] = '\x00'; } \
        { code } \
        start = end; \
    } \
}

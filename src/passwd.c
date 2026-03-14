#include <stdio.h>

#include "io/keyboard.h"
#include "hash/hash.h"
#include "types/string.h"
#include "passwd.h"


bool chk_passwd_set(const char* passwd_file) {
    FILE* file = fopen(passwd_file, "rb");
    if (file == NULL) { return false; }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    if (size == -1) { fclose(file); perror("File size unknown"); return false; }
    fclose(file);

    return ((size_t) size == SALT_SIZE + HASH_SIZE);
}


#define hash_password(salt_ptr, passwd_ptr) ({ \
    Bytes _tmp_concat_hash_pwd = bytes_concat(salt_ptr, &(passwd_ptr)->bytes); \
    Bytes _tmp_output_hash_pwd = bytes_sha512(&_tmp_concat_hash_pwd); \
    bytes_free(&_tmp_concat_hash_pwd); \
    _tmp_output_hash_pwd; \
})

bool chk_passwd(const char* prompt, const char* passwd_file) {
    String passwd = keyboard_ask_passwd(prompt);
    if (passwd.len == INVALID_SIZE) { return false; }

    FILE* file = fopen(passwd_file, "rb");
    if (file == NULL) {
        perror("Can't open password file");
        string_secure_free_force(&passwd);
        return false;
    }

    Bytes salt = bytes_file_read(file, SALT_SIZE, true);
    Bytes hash = bytes_file_read(file, HASH_SIZE, true);
    if ((salt.len != SALT_SIZE) || (hash.len != HASH_SIZE)) {
        string_secure_free_force(&passwd);
        bytes_free(&salt);
        bytes_free(&hash);
        return false;
    }
    fclose(file);

    Bytes hash_try = hash_password(&salt, &passwd);
    Success success = bytes_eq(&hash, &hash_try);
    string_secure_free_force(&passwd);
    bytes_free(&hash_try);
    bytes_free(&salt);
    bytes_free(&hash);
    return success;
}

Success set_passwd(const char* prompt, const char* passwd_file) {
    String passwd = keyboard_ask_passwd(prompt);
    if (passwd.len == INVALID_SIZE) { return false; }

    Bytes salt = bytes_from_random(SALT_SIZE);
    if (salt.len == 0) {
        fprintf(stderr, "set_passwd failed\n");
        string_free(&passwd);
        bytes_free(&salt);
        return false;
    }

    Bytes hash = hash_password(&salt, &passwd);
    string_free(&passwd);
    if (hash.len == 0) {
        fprintf(stderr, "set_passwd failed\n");
        bytes_free(&salt);
        bytes_free(&hash);
        return false;
    }

    FILE* file = fopen(passwd_file, "wb");
    if (file == NULL) {
        perror("fopen(<password file>) failed");
        bytes_free(&salt);
        bytes_free(&hash);
        return false;
    }

    fwrite(salt.data, 1, SALT_SIZE, file);
    fwrite(hash.data, 1, HASH_SIZE, file);
    bytes_free(&salt);
    bytes_free(&hash);
    fclose(file);

    return true;
}

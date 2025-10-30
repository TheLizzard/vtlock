#include <openssl/sha.h>
#include <openssl/evp.h>
#include <stdio.h>

#include "keyboard.h"
#include "passwd.h"


#define SALT_SIZE 64
#define HASH_SIZE SHA512_DIGEST_LENGTH


Bytes bytes_sha512(Bytes* input) {
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "EVP_MD_CTX_new failed\n");
        return bytes_empty();
    }
    if (EVP_DigestInit_ex(ctx, EVP_sha512(), NULL) != 1) {
        fprintf(stderr, "EVP_DigestInit_ex failed\n");
        EVP_MD_CTX_free(ctx);
        return bytes_empty();
    }
    if (EVP_DigestUpdate(ctx, input->data, input->len) != 1) {
        fprintf(stderr, "EVP_DigestUpdate failed\n");
        EVP_MD_CTX_free(ctx);
        return bytes_empty();
    }
    uint8_t* data = (uint8_t*) malloc(HASH_SIZE);
    if (EVP_DigestFinal_ex(ctx, data, NULL) != 1) {
        fprintf(stderr, "EVP_DigestFinal_ex failed\n");
        EVP_MD_CTX_free(ctx);
        free(data);
        return bytes_empty();
    }
    EVP_MD_CTX_free(ctx);
    return bytes_from_heap_data(data, HASH_SIZE);
}

Bytes bytes_from_random(size_t size) {
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (urandom == NULL) {
        perror("fopen(\"/dev/urandom\") failed");
        return bytes_empty();
    }
    Bytes output = bytes_file_read(urandom, size, true);
    if (output.len != size) {
        bytes_free(&output);
        fclose(urandom);
        return bytes_empty();
    }
    return output;
}


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
    Bytes _tmp_concat_hash_pwd = bytes_concat(salt_ptr, passwd_ptr); \
    Bytes _tmp_output_hash_pwd = bytes_sha512(&_tmp_concat_hash_pwd); \
    bytes_free(&_tmp_concat_hash_pwd); \
    _tmp_output_hash_pwd; \
})

bool chk_passwd(const char* prompt, const char* passwd_file) {
    Bytes passwd = keyboard_ask_passwd(prompt);

    FILE* file = fopen(passwd_file, "rb");
    if (file == NULL) { perror("Can't open password file"); return false; }

    Bytes salt = bytes_file_read(file, SALT_SIZE, true);
    Bytes hash = bytes_file_read(file, HASH_SIZE, true);
    if ((salt.len != SALT_SIZE) || (hash.len != HASH_SIZE)) { return false; }
    fclose(file);

    Bytes hash_try = hash_password(&salt, &passwd);
    Success success = bytes_eq(&hash, &hash_try);
    bytes_free(&passwd);
    bytes_free(&salt);
    bytes_free(&hash);
    bytes_free(&hash_try);
    return success;
}

Success set_passwd(const char* prompt, const char* passwd_file) {
    Bytes passwd = keyboard_ask_passwd(prompt);

    Bytes salt = bytes_from_random(SALT_SIZE);
    if (salt.len == 0) {
        fprintf(stderr, "set_passwd failed\n");
        bytes_free(&passwd);
        return false;
    }

    Bytes hash = hash_password(&salt, &passwd);
    if (hash.len == 0) {
        fprintf(stderr, "set_passwd failed\n");
        bytes_free(&passwd);
        return false;
    }
    bytes_free(&passwd);

    FILE* file = fopen(passwd_file, "wb");
    if (file == NULL) { perror("fopen(<password file>) failed"); return false; }

    fwrite(salt.data, 1, SALT_SIZE, file);
    fwrite(hash.data, 1, HASH_SIZE, file);
    bytes_free(&salt);
    bytes_free(&hash);
    fclose(file);

    return true;
}

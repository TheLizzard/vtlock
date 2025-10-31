#include <openssl/evp.h>

#include "hash.h"


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

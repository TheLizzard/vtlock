#pragma once
#include <openssl/sha.h>

#include "../types/bytes.h"


#define SALT_SIZE 64
#define HASH_SIZE SHA512_DIGEST_LENGTH


Bytes bytes_sha512(Bytes* input);
Bytes bytes_from_random(size_t size);

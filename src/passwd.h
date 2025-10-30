#pragma once
#include "bytes.h"

bool chk_passwd_set(const char* passwd_file);
Success set_passwd(const char* prompt, const char* passwd_file);
Success chk_passwd(const char* prompt, const char* passwd_file);

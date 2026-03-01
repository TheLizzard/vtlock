#pragma once
#include <stdbool.h>
#include <assert.h>

typedef bool Success;
#define panic() assert(false && "panic() called")

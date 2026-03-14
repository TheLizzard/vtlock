#pragma once
#include <stddef.h>

#include "success.h"


// Size is in bytes (look at comment above secure_delete.c@list_safe_clear_mem)
Success mem_secure_delete(volatile void* mem, size_t size);

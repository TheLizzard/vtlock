#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdint.h>

#include "success.h"


// explicit_bzero isn't in the C specification
// memset_s, memset_explicit aren't implemented by GNU C Library
// So we have to use `memset` and pray the optimiser is dumb enough
Success list_safe_clear_mem(volatile void* mem, size_t size) {
    if ((size == 0) || (mem == NULL)) { return false; }
    volatile uint8_t* p = (volatile uint8_t*) mem;
    while (size--) { *p++ = 0; }
    return true;
}

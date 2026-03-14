#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdint.h>

#include "secure_delete.h"


// explicit_bzero isn't in the C specification
// memset_s, memset_explicit aren't implemented by GNU C Library
// So we have to use `memset`
Success mem_secure_delete(volatile void* mem, size_t size) {
    if ((size == 0) || (mem == NULL)) { return false; }
    volatile uint8_t* p = (volatile uint8_t*) mem;
    while (size--) { *p++ = 0; }

    /*
    This here is a compiler fence to stop it from optimising out the
      loop above. Something simmilar is used in the Linux kernel so
      I will assume that it's good enough. For more info, look at:
    * https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/
                                    linux.git/tree/include/linux/compiler.h#n82
        The comment and issue explain the difference between barrier and
        barrier_data. The linked LLVM issue:
    * https://bugs.llvm.org/show_bug.cgi?id=15495#c11
        Talks about the difference between the `"g"` and `"r"` in `"+g"(p)`
    * https://github.com/llvm/llvm-project/pull/83577/files
        The implementation of `memset_explicit` in LLVM
    * https://discourse.llvm.org/t/implement-memset-explicit/77312
        More discussion on LLVM's `memset_explicit`
    * https://elixir.bootlin.com/glibc/glibc-2.42.9000/
                                    source/string/explicit_bzero.c
        GNU's implementation of `explicit_bzero`
    */
    __asm__ __volatile__("" : "+g"(p) : : "memory");
    return true;
}

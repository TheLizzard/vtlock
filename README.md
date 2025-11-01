# vtlock
Lock all ttys and display a clock until the user types in their password.

This was made to run in an initramfs environment.

### Compile:
Compile with your favourite C compiler using:
* C standard: `17`
* POSIX version: `200809L`
* Link against: OpenSSL (specifically `libssl` and `libcrypto`)

GCC command: `gcc $(find . -name '*.c') -std=c17 -D_POSIX_C_SOURCE=200809L -L/usr/lib/x86_64-linux-gnu -lssl -lcrypto`

### Tested:
The code has been tested on `gcc` with:
* `-Wall`
* `-Warray-bounds`
* `-Wconversion`
* `-Wdangling-else`
* `-Wextra`
* `-Wformat-security`
* `-Wnull-dereference`
* `-Wpointer-arith`
* `-Wshadow`
* `-Wswitch-enum`
* `-Wuninitialized`
* `-fsanitize-address-use-after-scope`
* `-fsanitize=address`
* `-fsanitize=leak`
* `-fsanitize=undefined`

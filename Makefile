TARGET      := vtlock
VERSION     := $(shell python3 -c 'print(__import__("json").load(open("metadata.json", "r"))["version"])')
AUTHOR      := $(shell python3 -c 'print(__import__("json").load(open("metadata.json", "r"))["author"])')
ARCH        := $(shell dpkg --print-architecture)

.ONESHELL:
SHELL       := /bin/bash
.SHELLFLAGS := -euo pipefail -c

PREFIX      := /usr
BINDIR      := $(PREFIX)/bin

BUILD_DIR   := build
PKG_DIR     := $(BUILD_DIR)/pkg
BIN         := $(BUILD_DIR)/$(TARGET)

PKG_NAME    := $(BUILD_DIR)/$(TARGET)_$(VERSION)_$(ARCH).deb
PKG_ROOT    := $(PKG_DIR)/$(TARGET)_$(VERSION)

# Detect pkg-config
PKG_CONFIG := $(shell command -v pkg-config 2>/dev/null)
ifeq ($(PKG_CONFIG),)
	$(error pkg-config is required. Install it with: sudo apt install pkg-config)
endif


# Colours
RESET=\x1b[0m
YELLOW=\x1b[33m
GREY=\x1b[90m
RED=\x1b[91m
GREEN=\x1b[92m
ORANGE=\x1b[93m
PURPLE=\x1b[95m
CYAN=\x1b[96m


# Detect compiler
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || \
        (command -v gcc >/dev/null 2>&1 && echo gcc || \
        (echo "Error: Can't find gcc or clang" && exit 1)))

# Recursively find sources
SRC := $(shell find src -type f -name '*.c')
OBJ := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC))

# Base flags
BASE_CFLAGS := -pipe -std=c17 -D_POSIX_C_SOURCE=200809L

BASE_CFLAGS_OPTIMISATION := \
-O3 -ffast-math -fomit-frame-pointer -funroll-loops -fno-math-errno \
-fstrict-aliasing -funsafe-math-optimizations -freciprocal-math \
-fno-trapping-math -finline-functions -fno-math-errno -ftree-vectorize \
-fno-asynchronous-unwind-tables -fmerge-all-constants -fvisibility=hidden
# cland flags
CLANG_CFLAGS := \
-flto=full -fno-common -ffunction-sections -fdata-sections -fvectorize \
-fomit-frame-pointer -falign-functions
# gcc flags
GCC_CFLAGS := \
-flto -fgraphite -fgraphite-identity -floop-nest-optimize -funroll-all-loops \
-fipa-pta -fno-strict-aliasing -frename-registers -funswitch-loops -fivopts \
-floop-block -floop-strip-mine -fno-common \
-fno-trapping-math


# clang warnings
CLANG_WARN := \
-Wall -Wextra -Wshadow -Wdangling-else -Wswitch-enum -Wformat-security \
-Wconversion -Wpointer-arith -Wdouble-promotion -Wundef -Wfloat-equal \
-Warray-bounds -Wnull-dereference -Wnullable-to-nonnull-conversion \
-Wsometimes-uninitialized -Wcast-align -Wtautological-compare \
-Wbitwise-instead-of-logical -Wshorten-64-to-32 -Wconditional-uninitialized \
-Wdocumentation -Wreturn-stack-address -Wformat-pedantic \
-Wimplicit-int-float-conversion -Wcomma
# gcc warnings
GCC_WARN := \
-Wall -Walloc-zero -Warray-bounds -Wcast-align -Wconversion -Wdangling-else \
-Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra \
-Wfloat-equal -Wformat-security -Wlogical-op -Wnull-dereference \
-Wpointer-arith -Wshadow -Wswitch-enum -Wundef -Wuninitialized


# Select flags based on compiler
CFLAGS := $(BASE_CFLAGS) $(BASE_CFLAGS_OPTIMISATION)
CFLAGS_PP := $(PURPLE)$(BASE_CFLAGS)$(RESET) $(YELLOW)$(BASE_CFLAGS_OPTIMISATION)$(RESET)
ifeq ($(CC),clang)
	CFLAGS += $(CLANG_CFLAGS) $(CLANG_WARN)
	CFLAGS_PP += $(ORANGE)$(CLANG_CFLAGS)$(RESET) $(GREY)$(CLANG_WARN)$(RESET)
else
	CFLAGS += $(GCC_CFLAGS) $(GCC_WARN)
	CFLAGS_PP += $(ORANGE)$(GCC_CFLAGS)$(RESET) $(GREY)$(GCC_WARN)$(RESET)
endif


# Static linking
LDFLAGS := -static -flto
LDLIBS := -lssl -lcrypto -lz -lzstd -ldl -lpthread -lm

# Ignore warnings about statically linking libcrypto
EXCLUDE_WARNINGS = -e "statically linked applications" -e "libcrypto.a"


define CONTROL_FILE
Package: $(TARGET)
Version: $(VERSION)
Section: utils
Priority: optional
Architecture: $(ARCH)
Maintainer: $(AUTHOR)
Description: Virtual terminal locking utility
 vtlock is a small utility that locks the current Linux virtual terminal
 (TTY) and prevents switching to other terminals while it is active.
 .
 When started, the program asks for the lock password to confirm that the
 configured password file is valid. Once locked, the terminal cannot be
 exited with common signals such as Ctrl-C or Ctrl-Z and virtual terminal
 switching is disabled.
 .
 The screen displays a simple clock-style screensaver while the terminal
 is locked. To unlock the terminal, the correct password must be entered.
 The program reports the number of incorrect password attempts after a
 successful unlock.
endef

define CONTROL_FILE_ADD_ON
Note - vtlock requires CAP_SYS_ADMIN to disable VT switching.
It can be run as root, or the package maintainer can set the
capability with 'setcap cap_sys_admin+ep /usr/bin/vtlock'.
endef


.PHONY: all check-deps build clean install uninstall package

all: build

check-deps:
	@pkg-config --exists zlib || (echo "Missing dependency: zlib1g-dev"; exit 1)
	@pkg-config --exists libzstd || (echo "Missing dependency: libzstd-dev"; exit 1)
	@pkg-config --exists openssl || (echo "Missing dependency: libssl-dev"; exit 1)

build: check-deps print-compiler $(BIN)

print-compiler:
	@echo -e "$(CYAN)Using these $(GREEN)$(CC)$(CYAN) flags:$(RESET) $(CFLAGS_PP)"

$(BIN): $(OBJ) metadata.json
	@mkdir -p $(dir $@)
	@echo -e "$(CYAN)Compiling into $(GREEN)$@$(RESET)"
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS) 2> >(grep -v $(EXCLUDE_WARNINGS))

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo -e "$(CYAN)Compiling: $(GREEN)$<$(RESET)"
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(PKG_DIR) *.deb

install: $(BIN)
	@install -Dm755 $(BIN) $(DESTDIR)$(BINDIR)/$(TARGET)

uninstall:
	@rm -f $(DESTDIR)$(BINDIR)/$(TARGET)

package: build
	@rm -rf $(PKG_DIR)
	@mkdir -p $(PKG_ROOT)/DEBIAN
	@mkdir -p $(PKG_ROOT)$(BINDIR)

	@install -m755 $(BIN) $(PKG_ROOT)$(BINDIR)/$(TARGET)
	@printf "%s\n" "$(CONTROL_FILE)" > $(PKG_ROOT)/DEBIAN/control
	@sed -i '/^[A-Za-z]\+: .*/! s/^/ /' $(PKG_ROOT)/DEBIAN/control
	@dpkg-deb --build --root-owner-group $(PKG_ROOT)

	@mv $(PKG_DIR)/$(TARGET)_$(VERSION).deb $(PKG_NAME)

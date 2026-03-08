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


# Detect compiler
CC := $(shell command -v clang >/dev/null 2>&1 && echo clang || echo gcc)

# Recursively find sources
SRC := $(shell find src -type f -name '*.c')
OBJ := $(patsubst ./%.c,$(BUILD_DIR)/%.o,$(SRC))

# Base flags
BASE_CFLAGS := -pipe -Ofast -flto -march=native -mtune=native -fomit-frame-pointer \
               -funroll-loops -funroll-all-loops -fstrict-aliasing \
               -ffast-math -fno-math-errno -funsafe-math-optimizations \
               -fno-trapping-math -freciprocal-math \
               -finline-functions -finline-functions-called-once \
               -fipa-pta -fgraphite -fdevirtualize -frename-registers \
               -D_POSIX_C_SOURCE=200809L -std=c17

# Clang warnings
CLANG_WARN := \
-Wall -Wextra -Wshadow -Wdangling-else -Wswitch-enum -Wformat-security \
-Wconversion -Wpointer-arith -Wdouble-promotion -Wundef -Wfloat-equal \
-Warray-bounds -Wnull-dereference -Wnullable-to-nonnull-conversion \
-Wsometimes-uninitialized -Wcast-align -Wtautological-compare \
-Wbitwise-instead-of-logical -Wshorten-64-to-32 -Wconditional-uninitialized \
-Wdocumentation -Wreturn-stack-address -Wformat-pedantic \
-Wcovered-switch-default -Wimplicit-int-float-conversion -Wcomma

# GCC warnings
GCC_WARN := \
-Wall -Walloc-zero -Warray-bounds -Wcast-align -Wconversion -Wdangling-else \
-Wdouble-promotion -Wduplicated-branches -Wduplicated-cond -Wextra \
-Wfloat-equal -Wformat-security -Wlogical-op -Wnull-dereference \
-Wpointer-arith -Wshadow -Wswitch-enum -Wundef -Wuninitialized

# Select warnings based on compiler
ifeq ($(CC),clang)
WARNINGS := $(CLANG_WARN)
else
WARNINGS := $(GCC_WARN)
BASE_CFLAGS += -fgraphite -fgraphite-identity -floop-nest-optimize
endif

CFLAGS := $(BASE_CFLAGS) $(WARNINGS)

# Static linking
LDFLAGS := -static -flto
LDLIBS := -lssl -lcrypto -lz -lzstd -ldl -lpthread -lm


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

build: check-deps $(BIN)

$(BIN): $(OBJ) metadata.json
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LDLIBS) 2> >(grep -v "statically linked applications")

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

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

# (C) 2018 Srimanta Barua
# Makefile for libc and libk

# Configuration options
ARCH?=x86_64
OPT?=-O2

# Fixing compiler to GCC for now..
CC:=$(ARCH)-tmos-gcc

# Compiler warnings
WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

# Compiler and linker flags
CFLAGS:=$(OPT) $(WARN)
CPPFLAGS:=-I../sysroot/usr/include/ -D__TMOS_CFG_ARCH_$(ARCH)__
LDFLAGS:=-nostdlib
LIBS:=

CFLAGS.libc:=$(CFLAGS) -fPIC
CPPFLAGS.libc:=$(CPPFLAGS)
LDFLAGS.libc:=$(LDFLAGS)
LIBS.libc:=$(LIBS)

CFLAGS.libk:=$(CFLAGS) -ffreestanding
CPPFLAGS.libk:=$(CPPFLAGS) -D__TMOS_KERNEL__
LDFLAGS.libk:=$(LDFLAGS)
LIBS.libk:=$(LIBS)

# Directories
BUILD.libc:=build_libc
BUILD.libk:=build_libk

# Output
LIBC:=$(BUILD.libc)/libc.a
LIBK:=$(BUILD.libk)/libk.a

# Objects
SRC:=$(wildcard *.c)
OBJS.libc:=$(patsubst %.c,$(BUILD.libc)/%.o,$(SRC))
OBJS.libk:=$(patsubst %.c,$(BUILD.libk)/%.o,$(SRC))

.PHONY: libc libk clean

libc: $(LIBC)

libk: $(LIBK)

$(LIBC): $(OBJS.libc)
	$(AR) rcs $@ $^
	make -C arch/$(ARCH)/crt

$(LIBK): $(OBJS.libk)
	$(AR) rcs $@ $^

clean:
	rm -rf $(BUILD.libc) $(BUILD.libk)
	make -C arch/$(ARCH)/crt clean

build_libc/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS.libc) $(CPPFLAGS.libc)

build_libk/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c -o $@ $< $(CFLAGS.libk) $(CPPFLAGS.libk)

# (C) 2018 Srimanta Barua
# Makefile for the shuos kernel

# Configuration options
ARCH?=x86_64
OPT?=-O2

# We're fixing the compiler to GCC for now..
CC:=$(ARCH)-elf-gcc

# Compiler warnings
WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

# Compiler and linker flags
CFLAGS:=-ffunction-sections -fdata-sections -ffreestanding $(OPT) $(WARN)
CPPFLAGS:=-I../include/ -D__CFG_ARCH_$(ARCH)__
LDFLAGS:=-nostdlib -Wl,-gc-sections
LIBS:=-lgcc

# The kernel ELF binary
KERNEL:=shuos.kernel

# Objects which will be linked to make the kernel binary
SRC_OBJS:=klog.o ctype.o vsprintf.o string.o multiboot2.o mem/memory.o mem/bitmap.o mem/heap.o
CRTBEGIN_OBJ:=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) -print-file-name=crtend.o)

# Include arch-specific config
include arch/$(ARCH)/make.config

# Add arch-specific compiler and linker flags
CFLAGS+=$(ARCH_CFLAGS)
CPPFLAGS+=$(ARCH_CPPFLAGS)
LDFLAGS+=$(ARCH_LDFLAGS)
LIBS+=$(ARCH_LIBS)

# Add arch specific objects and build list of objects to be linked
SRC_OBJS+=$(ARCH_OBJS)
BUILD_OBJS:=$(ARCH_CRTI_OBJ) $(SRC_OBJS) $(ARCH_CRTN_OBJ)
TOTAL_OBJS:=$(ARCH_CRTI_OBJ) $(CRTBEGIN_OBJ) $(SRC_OBJS) $(CRTEND_OBJ) $(ARCH_CRTN_OBJ)


.PHONY: all clean test_grub purge

# Build the kernel
all: $(KERNEL)

# Clean kernel and object files
clean:
	rm -rf $(KERNEL) $(BUILD_OBJS)

# Run a script to create a dummy ISO with grub-mkrescue, and run using QEMU
# TODO: Better testing, especially when we have userspace
test_grub: all
	./scripts/test_grub.sh run

# Clean, plus also clean the results of the test_grub script
purge: clean
	./scripts/test_grub.sh clean

# Link the kernel
$(KERNEL): $(BUILD_OBJS)
	@echo "  LD $@"
	@$(CC) -n -T $(ARCH_LDSCRIPT) -o $@ $(TOTAL_OBJS) $(LDFLAGS) $(LIBS)

# Compile C objects
%.o: %.c
	@echo "  CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

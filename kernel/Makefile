# (C) 2018 Srimanta Barua
# Makefile for the tmos kernel

# Configuration options
ARCH?=x86_64
OPT?=-O2

# We're fixing the compiler to GCC for now..
CC:=$(ARCH)-tmos-gcc

# Compiler warnings
WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

# Compiler and linker flags
CFLAGS:=-ffunction-sections -fdata-sections -ffreestanding $(OPT) $(WARNINGS)
CPPFLAGS:=-I../sysroot/usr/include/ -D__TMOS_CFG_ARCH_$(ARCH)__ -D__TMOS_KERNEL__
LDFLAGS:=-nostdlib -Wl,-gc-sections -L.
LIBS:=-lk

# The kernel ELF binary
KERNEL:=tmos.kernel

# Objects which will be linked to make the kernel binary
OBJS:=klog.o vsprintf.o multiboot2.o mem/memory.o mem/bitmap.o mem/heap.o

# Include arch-specific config
include arch/$(ARCH)/make.config

# Add arch-specific compiler and linker flags
CFLAGS+=$(ARCH_CFLAGS)
CPPFLAGS+=$(ARCH_CPPFLAGS)
LDFLAGS+=$(ARCH_LDFLAGS)
LIBS+=$(ARCH_LIBS)

# Add arch specific objects and build list of objects to be linked
OBJS+=$(ARCH_OBJS)

# Kernel's statically-compiled version of libk
LIBK=libk.a


.PHONY: all clean test_grub purge libk

# Build the kernel
all: $(KERNEL)

# Clean kernel and object files
clean:
	rm -rf $(KERNEL) $(OBJS) $(LIBK)

# Run a script to create a dummy ISO with grub-mkrescue, and run using QEMU
# TODO: Better testing, especially when we have userspace
test_grub: all
	./scripts/test_grub.sh run

# Clean, plus also clean the results of the test_grub script
purge: clean
	./scripts/test_grub.sh clean

libk:
	make -C ../libc libk
	cp ../libc/build_libk/libk.a $(LIBK)

# Link the kernel
$(KERNEL): $(OBJS) libk
	@echo "  LD $@"
	@$(CC) -n -T $(ARCH_LDSCRIPT) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

# Compile C objects
%.o: %.c
	@echo "  CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

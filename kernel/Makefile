# Makefile for the shuos kernel

ARCH?=x86_64
OPT?=-O2

CC:=$(ARCH)-elf-gcc

WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

CFLAGS:=-ffunction-sections -fdata-sections -ffreestanding $(OPT) $(WARN)
CPPFLAGS:=-Iinclude/ -D__ARCH_$(ARCH)__
LDFLAGS:=-nostdlib -Wl,-gc-sections
LIBS:=-lgcc

KERNEL:=shuos.kernel

SRC_OBJS:=klog.o ctype.o vsprintf.o string.o
CRTBEGIN_OBJ:=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) -print-file-name=crtend.o)

include arch/$(ARCH)/make.config

CFLAGS+=$(ARCH_CFLAGS)
CPPFLAGS+=$(ARCH_CPPFLAGS)
LDFLAGS+=$(ARCH_LDFLAGS)
LIBS+=$(ARCH_LIBS)

SRC_OBJS+=$(ARCH_OBJS)
BUILD_OBJS:=$(ARCH_CRTI_OBJ) $(SRC_OBJS) $(ARCH_CRTN_OBJ)
TOTAL_OBJS:=$(ARCH_CRTI_OBJ) $(CRTBEGIN_OBJ) $(SRC_OBJS) $(CRTEND_OBJ) $(ARCH_CRTN_OBJ)

.PHONY: all clean test_grub purge

all: $(KERNEL)

clean:
	rm -rf $(KERNEL) $(BUILD_OBJS)

test_grub: all
	./scripts/test_grub.sh run

purge: clean
	./scripts/test_grub.sh clean

$(KERNEL): $(BUILD_OBJS)
	@echo "  LD $@"
	@$(CC) -n -T $(ARCH_LDSCRIPT) -o $@ $(TOTAL_OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	@echo "  CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

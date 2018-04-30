# Makefile for the shuos kernel

ARCH?=x86_64
CC:=$(ARCH)-elf-gcc

WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

OPT:=-O2

CFLAGS:=-ffunction-sections -fdata-sections -mno-red-zone -mno-mmx -mno-sse -mno-sse2\
	-ffreestanding -masm=intel $(OPT) $(WARN)
CPPFLAGS:=-Iinclude/
LDFLAGS:=-nostdlib -Wl,-gc-sections
LIBS:=-lgcc

KERNEL:=shuos.kernel

SRC_OBJS:=
CRTBEGIN_OBJ:=$(shell $(CC) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) -print-file-name=crtend.o)

include arch/$(ARCH)/make.config

SRC_OBJS+=$(ARCH_OBJS)
LDFLAGS+=$(ARCH_LDFLAGS)
LIBS+=$(ARCH_LIBS)

BUILD_OBJS:=$(ARCH_CRTI_OBJ) $(SRC_OBJS) $(ARCH_CRTN_OBJ)
TOTAL_OBJS:=$(ARCH_CRTI_OBJ) $(CRTBEGIN_OBJ) $(SRC_OBJS) $(CRTEND_OBJ) $(ARCH_CRTN_OBJ)

.PHONY: all clean test_grub

all: $(KERNEL)

clean:
	rm -f $(KERNEL) $(BUILD_OBJS)

test_grub: all
	./scripts/test_grub.sh

$(KERNEL): $(BUILD_OBJS)
	@echo "  LD $@"
	@$(CC) -n -T $(ARCH_LDSCRIPT) -o $@ $(TOTAL_OBJS) $(LDFLAGS) $(LIBS)

%.o: %.c
	@echo "  CC $<"
	@$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

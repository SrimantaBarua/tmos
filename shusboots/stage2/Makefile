# Makefile for stage2 of the bootloader

ARCH=i686
CC=$(ARCH)-elf-gcc
LD=$(ARCH)-elf-gcc

WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

OPT:=-O2

CFLAGS:=-ffunction-sections -fdata-sections -mno-red-zone -mno-mmx -mno-sse -mno-sse2\
	-ffreestanding -masm=intel $(WARNINGS) $(OPT)
CPPFLAGS:=-Iinclude/
LDFLAGS:=-nostdlib -Wl,-gc-sections
LIBS:=-lgcc

LINKER_SCRIPT:=linker.ld

stage_2:=stage2.bin
stage_2_elf:=stage2.elf

asm_objs:=asm/entry.o
c_objs:=main.o mem.o io/serial.o io/port_io.o util/log.o util/fmt.o util/intutil.o util/string.o

crti_obj:=crt/crti.o
crtn_obj:=crt/crtn.o
crtbegin_obj:=$(shell $(CC) -print-file-name=crtbegin.o)
crtend_obj:=$(shell $(CC) -print-file-name=crtend.o)

objs:=$(crti_obj) $(crtbegin_obj) $(asm_objs) $(c_objs) $(crtend_obj) $(crtn_obj)

.PHONY: all clean

all: $(stage_2)

$(stage_2): $(stage_2_elf)
	objcopy -O binary $< $@

$(stage_2_elf): $(crti_obj) $(asm_objs) $(c_objs) $(crtn_obj)
	$(LD) -n -T $(LINKER_SCRIPT) -o $@ $(objs) $(LDFLAGS) $(LIBS)

clean:
	rm -f $(stage_2) $(stage_2_elf) $(crti_obj) $(asm_objs) $(c_objs) $(crtn_obj)

%.o: %.asm
	nasm -felf32 -o $@ $<

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

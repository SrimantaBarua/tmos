# Makefile for stage2 of the bootloader

ARCH=i686
CC=$(ARCH)-elf-gcc
LD=$(ARCH)-elf-ld

WARNINGS:=-Wall -Wextra -Wshadow -Wcast-align -Wwrite-strings -Wredundant-decls -Wnested-externs\
	-Wno-long-long -Winline -Wuninitialized

CFLAGS:=-Os -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -masm=intel $(WARNINGS)
CPPFLAGS:=
LDFLAGS:=-nostdlib
LIBS:=

LINKER_SCRIPT:=linker.ld

stage_2:=stage2.bin
stage_2_elf:=stage2.elf

asm_objs:=entry.o
c_objs:=main.o

crti_obj:=crti.o
crtn_obj:=crtn.o
crtbegin_obj:=$(shell $(CC) -print-file-name=crtbegin.o)
crtend_obj:=$(shell $(CC) -print-file-name=crtend.o)

objs:=$(crti_obj) $(crtbegin_obj) $(asm_objs) $(c_objs) $(crtend_obj) $(crtn_obj)

.PHONY: all clean purge

all: $(stage_2)

$(stage_2): $(stage_2_elf)
	objcopy -O binary $< $@

$(stage_2_elf): $(crti_obj) $(asm_objs) $(c_objs) $(crtn_obj)
	$(LD) -n -T $(LINKER_SCRIPT) -o $@ $(objs) $(LDFLAGS) $(LIBS) --gc-sections

clean:
	rm -f $(stage_2) $(crti_obj) $(asm_objs) $(c_objs) $(crtn_obj)

%.o: %.asm
	nasm -felf32 -o $@ $<

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

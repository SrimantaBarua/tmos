# Makefile for the Rust part of the bootloader
# Right now, tailored for the MBR-specific bootloader

stage_2:=stage2.bin
stage_2_elf:=stage2.elf
asm_obj:=src/entry.o
rust_obj:=target/i686-shusboots/release/libstage2.a

# Targets
.PHONY: all clean purge

all: $(stage_2)
	
$(stage_2): $(stage_2_elf)
	objcopy -O binary stage2.elf $@
	
$(stage_2_elf): $(asm_obj) $(rust_obj)
	i686-elf-ld -n -T linker.ld -o stage2.elf $^ --gc-sections

$(rust_obj):
	xargo build --release --target=i686-shusboots

%.o: %.asm
	nasm -felf32 -o $@ $<

clean:
	rm -f $(stage_2) $(asm_obj) $(stage_2_elf)

purge: clean
	xargo clean

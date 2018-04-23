# Makefile for the bootloader
# Right now, tailored for the MBR-specific bootloader

# Stage 1
stage_1=stage1.bin

# Targets
.PHONY: all clean

all: $(stage_1)

clean:
	rm -f $(stage_1)

# Assemling
$(stage_1): boot_mbr.asm
	nasm -f bin -o $@ $<

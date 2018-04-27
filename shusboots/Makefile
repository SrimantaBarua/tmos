# Makefile for the bootloader
# Right now, tailored for the MBR-specific bootloader

# Stage 1
stage_1:=stage1.bin
disk_img:=disk.img

# Targets
.PHONY: all clean purge run

all: $(disk_img)
	
$(disk_img): $(stage_1) $(stage_2)
	dd if=/dev/zero of=disk.img bs=512 count=2048
	dd if=$(stage_1) of=disk.img conv=notrunc

clean:
	rm -f $(stage_1)

purge: clean
	rm -f $(disk_img)

run: all
	qemu-system-x86_64 $(disk_img)

# Assembling
$(stage_1): stage1/boot_mbr.asm
	nasm -f bin -o $@ $<

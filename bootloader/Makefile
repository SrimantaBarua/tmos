# Makefile for the bootloader
# Right now, tailored for the MBR-specific bootloader

stage_1:=stage1.bin
stage_2:=stage2.bin
disk_img:=disk.img

# Targets
.PHONY: all clean purge run stage_2

all: $(disk_img)
	
$(disk_img): $(stage_1) stage_2
	dd if=/dev/zero of=disk.img bs=512 count=2048
	dd if=$(stage_1) of=disk.img conv=notrunc
	dd if=$(stage_2) of=disk.img conv=notrunc bs=512 seek=4

clean:
	rm -f $(stage_1) $(stage_2)
	make -C stage2 clean

purge: clean
	rm -f $(disk_img)

run: all
	qemu-system-x86_64 $(disk_img) -serial file:serial.log

stage_2:
	make -C stage2
	cp stage2/stage2.bin .

# Assembling
$(stage_1): stage1/boot_mbr.asm
	nasm -f bin -o $@ $<

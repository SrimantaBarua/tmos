# Makefile for the bootloader
# Right now, tailored for the MBR-specific bootloader


stage_1:=stage1.bin
stage_2:=stage2.bin
disk_img:=disk.img

serial_file?=serial.log

sb_cfg=boot.cfg


# Targets
.PHONY: all clean purge run stage_2


all: $(disk_img)

	
$(disk_img): $(stage_1) stage_2
	dd if=/dev/zero of=$(disk_img) bs=4096 count=512
	parted --script $(disk_img) \
		mklabel msdos \
		mkpart primary fat32 1MiB 100% \
		set 1 boot on
	dd if=$(stage_1) of=$(disk_img) conv=notrunc bs=446 count=1
	dd if=$(stage_2) of=$(disk_img) conv=notrunc bs=512 seek=1 count=24
	dd if=/dev/zero of=tmp_disk bs=4096 count=1792
	mkfs.fat -F12 tmp_disk -n BOOTVOL
	mcopy -i tmp_disk $(sb_cfg) ::/
	dd if=tmp_disk of=$(disk_img) conv=notrunc bs=4096 seek=256
	rm -f tmp_disk


clean:
	rm -f $(stage_1) $(stage_2)
	make -C stage2 clean


purge: clean
	rm -f $(disk_img)


run: all
	qemu-system-x86_64 -hda $(disk_img) -serial file:$(serial_file)


stage_2:
	make -C stage2
	cp stage2/stage2.bin .


# Assembling
$(stage_1): stage1/boot_mbr.asm
	nasm -f bin -o $@ $<

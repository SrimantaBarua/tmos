#!/bin/bash

iso="shuos_kernel_test.iso"
kernel="shuos.kernel"
serial_cmd="-serial file:serial.log"

mkdir -p isodir/boot/grub
cat > isodir/boot/grub/grub.cfg << EOF
set timeout=0
set default=0
menuentry "shuos" {
	multiboot2 /boot/shuos.kernel
	boot
}
EOF
cp $kernel isodir/boot/
grub-mkrescue -o $iso isodir -d /usr/lib/grub/i386-pc
qemu-system-x86_64 $iso $serial_cmd -no-reboot

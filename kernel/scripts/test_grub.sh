#!/bin/bash

if [ $# == 0 ]; then
	echo "USAGE: $0 [run | clean]"
	exit 1
fi

iso="shuos_kernel_test.iso"
kernel="shuos.kernel"
serial_file="serial.log"
serial_cmd="-serial file:$serial_file"

function grub_run() {
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
qemu-system-x86_64 $iso $serial_cmd
}

function grub_clean() {
rm -rf $iso $kernel $serial_file isodir
}

case "$1" in
"run")
	grub_run ;;
"clean")
	grub_clean ;;
*)
	echo "Unknown option: $1";
	exit 1 ;;
esac

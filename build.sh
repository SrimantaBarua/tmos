#!/bin/bash

# ---- Settings -----

binutils_ver="2.31"
gcc_ver="8.2.0"
autoconf_ver="2.69"
arch="x86_64"

cores=$(grep processor /proc/cpuinfo | wc -l)

# -------------------

cur_dir=$PWD
sysroot=$cur_dir/sysroot
prefix_dir=$sysroot/usr
include_dir=$prefix_dir/include
lib_dir=$prefix_dir/lib
boot_dir=$sysroot/boot
grub_dir=$boot_dir/grub

tools_dir=$cur_dir/tools
tools_src_dir=$tools_dir/src
tools_patch_dir=$tools_dir/patches
tools_cross_dir=$tools_dir/cross
gnu_url="https://ftp.gnu.org/gnu"

kernel_dir=$cur_dir/kernel
kernel=tmos.kernel

libc_dir=$cur_dir/libc
libc=$libc_dir/build_libc/libc.a
crtobjs="$libc_dir/arch/$arch/crt/crt0.o $libc_dir/arch/$arch/crt/crti.o $libc_dir/arch/$arch/crt/crtn.o"

iso=$cur_dir/tmos_kernel_test.iso

serial_file=$cur_dir/serial.log

TARGET=$arch-tmos
PREFIX=$tools_cross_dir
PATH=$tools_cross_dir/bin:$PATH

# -------------------

check_autoconf_version() {
	ver=$(autoconf --version | head -n 1 | cut -d ' ' -f 4)
	if [ $ver != $autoconf_ver ] ; then
		echo "Please use Autoconf version exactly 2.64"
		echo "This is a requirement for building patched GCC"
		exit 1
	fi
}

build_autoconf() {
	cd $cur_dir
	echo  -e "\n======== BUILD AUTOCONF ========"
	mkdir -p $tools_src_dir
	cd $tools_src_dir
	if [ ! -f "autoconf-$autoconf_ver.tar.xz" ]; then
		echo -e "\n---------- Downloading ---------"
		wget -O "autoconf-$autoconf_ver.tar.xz" "$gnu_url/autoconf/autoconf-$autoconf_ver.tar.xz"
		echo "------------- Done -------------"
	fi
	echo -e "\n* autoconf-$autoconf_ver.tar.xz downloaded"
	if [ ! -d "autoconf-$autoconf_ver" ]; then
		echo -e "\n---------- Extracting ----------"
		tar -xf "autoconf-$autoconf_ver.tar.xz"
		echo "------------- Done -------------"
	fi
	echo -e "\n* autoconf-$autuconf_ver extracted"
	echo -e "\n----------- Building -----------"
	cd autoconf-$autoconf_ver
	./configure
	make -j$cores
	sudo make install
	echo "------------- Done -------------"
	echo -e "\n* autoconf-$autoconf_ver built"
	echo -e "\n============= DONE =============="
}

copy_headers() {
	cd $cur_dir
	if [ ! -d $include_dir ]; then
		cp -R $cur_dir/include $include_dir
		cd $include_dir/tmos
		mv arch/$arch arch_$arch
		rm -r arch
		mv arch_$arch arch
	fi
}

build_copy_crtstuff() {
	cd $cur_dir
	echo  -e "\n======== BUILD CRTSTUFF ========\n"
	mkdir -p $lib_dir
	make -C $cur_dir/libc/arch/$arch/crt
	cp $cur_dir/libc/arch/$arch/crt/*.o $lib_dir
	echo -e "\n============= DONE =============="
}

build_patched_binutils() {
	echo  -e "\n======== BUILD BINUTILS ========"
	mkdir -p $tools_src_dir
	cd $tools_src_dir
	if [ ! -f "binutils-$binutils_ver.tar.xz" ]; then
		echo -e "\n---------- Downloading ---------"
		wget -O "binutils-$binutils_ver.tar.xz" "$gnu_url/binutils/binutils-$binutils_ver.tar.xz"
		rm -f $tools_src_dir/.binutils_patched
		echo "------------- Done -------------"
	fi
	echo -e "\n* binutils-$binutils_ver.tar.xz downloaded"
	if [ ! -d "binutils-$binutils_ver" ]; then
		echo -e "\n---------- Extracting ----------"
		tar -xf "binutils-$binutils_ver.tar.xz"
		rm -f $tools_src_dir/.binutils_patched
		echo "------------- Done -------------"
	fi
	echo -e "\n* binutils-$binutils_ver extracted"
	if [ ! -f .binutils_patched ]; then
		echo -e "\n----------- Patching -----------"
		cd $tools_src_dir/binutils-$binutils_ver
		patch -p1 < $tools_patch_dir/binutils-$binutils_ver.patch
		touch $tools_src_dir/.binutils_patched
		echo "------------- Done -------------"
	fi
	cd $tools_src_dir
	echo -e "\n* binutils-$binutils_ver patched"
	if [ -z "$(command -v $TARGET-ld)" ]; then
		echo -e "\n----------- Building -----------"
		rm -rf $tools_src_dir/build-binutils
		mkdir -p $tools_src_dir/build-binutils
		cd $tools_src_dir/build-binutils
		../binutils-$binutils_ver/configure --target="$TARGET" --prefix="$PREFIX" --with-sysroot="$sysroot/" --disable-nls --disable-werror
		make -j$cores
		make install
		echo "------------- Done -------------"
	fi
	echo -e "\n* binutils-$binutils_ver built"
	echo -e "\n============= DONE =============="
}

build_patched_gcc() {
	echo  -e "\n=========== BUILD GCC =========="
	mkdir -p $tools_src_dir
	cd $tools_src_dir
	if [ ! -f "gcc-$gcc_ver.tar.xz" ]; then
		echo -e "\n---------- Downloading ---------"
		wget -O "gcc-$gcc_ver.tar.xz" "$gnu_url/gcc/gcc-$gcc_ver/gcc-$gcc_ver.tar.xz"
		rm -f $tools_src_dir/.gcc_patched
		echo "------------- Done -------------"
	fi
	echo -e "\n* gcc-$gcc_ver.tar.xz downloaded"
	if [ ! -d "gcc-$gcc_ver" ]; then
		echo -e "\n---------- Extracting ----------"
		tar -xf "gcc-$gcc_ver.tar.xz"
		rm -f $tools_src_dir/.gcc_patched
		echo "------------- Done -------------"
	fi
	echo -e "\n* gcc-$gcc_ver extracted"
	if [ ! -f .gcc_patched ]; then
		echo -e "\n----------- Patching -----------"
		cd $tools_src_dir/gcc-$gcc_ver
		patch -p1 < $tools_patch_dir/gcc-$gcc_ver.patch
		touch $tools_src_dir/.gcc_patched
		echo "------------- Done -------------"
	fi
	cd $tools_dir_dir
	echo -e "\n* gcc-$gcc_ver patched"
	if [ -z "$(command -v $TARGET-gcc)" ]; then
		echo -e "\n----------- Building -----------"
		rm -rf $tools_src_dir/build-gcc
		mkdir $tools_src_dir/build-gcc
		cd $tools_src_dir/build-gcc
		../gcc-$gcc_ver/configure --target="$TARGET" --prefix="$PREFIX" --disable-nls --enable-languages=c --with-sysroot="$sysroot/" --disable-multilib
		make all-gcc -j$cores
		make all-target-libgcc -j$cores
		make install-gcc
		make install-target-libgcc
		echo "------------- Done -------------"
	fi
	echo -e "\n* gcc-$gcc_ver built"
	echo -e "\n============= DONE =============="
}

build_libc() {
	cd $cur_dir
	mkdir -p $lib_dir
	make -C $libc_dir libc
	cp $libc $lib_dir
	cp $crtobjs $lib_dir
}

build_kernel() {
	cd $cur_dir
	make -C $kernel_dir
}

system() {
	# 1. Make tools directory
	mkdir -p $tools_dir
	mkdir -p $tools_src_dir

	# 2. Check autoconf version
	check_autoconf_version

	# 3. Create sysroot directory structure
	mkdir -p $prefix_dir
	mkdir -p $lib_dir
	mkdir -p $boot_dir

	# 4. Copy headers to sysroot
	copy_headers
	
	# 5. Build and copy crt*.o
	build_copy_crtstuff

	# 6. Build patched binutils
	build_patched_binutils

	# 7. Build patched gcc
	build_patched_gcc

	# 8. Build and install libc
	build_libc

	# 9. Build and install kernel
	build_kernel
}

iso() {
	# Build system
	system

	# Build ISO
	mkdir -p $grub_dir
	cat > $grub_dir/grub.cfg << EOF
set timeout=0
set default=0
menuentry "tmos" {
	insmod gzio
	multiboot2 /boot/tmos.kernel.gz
	boot
}
EOF
	cp $kernel_dir/$kernel $boot_dir
	gzip $boot_dir/$kernel -f
	grub-mkrescue -o $iso $sysroot -d /usr/lib/grub/i386-pc
	qemu-system-x86_64 $iso -serial file:$serial_file -no-reboot -d int --enable-kvm -cpu host
}

clean() {
	make -C $kernel_dir clean
	make -C $libc_dir clean
	rm -rf $serial_file $iso $sysroot
}

purge() {
	clean
	rm -rf $tools_src_dir $tools_cross_dir
}

print_usage() {
	echo "USAGE: ./build.sh [ check_autoconf | build_autoconf | system | iso | clean | purge ]"
}


case $# in
	0)
		system
		;;
	1)
		case $1 in
			check_autoconf)
				check_autoconf_version
				echo "* Valid autoconf version ($autoconf_ver) installed"
				;;
			build_autoconf)
				build_autoconf
				;;
			system)
				system
				;;
			iso)
				iso
				;;
			clean)
				clean
				;;
			purge)
				purge
				;;
			*)
				print_usage
				exit 1
				;;
		esac
		;;
	*)
		print_usage
		exit 1
		;;
esac

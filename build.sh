#!/usr/bin/env bash

gcc_ver="8.2.0"
binutils_ver="2.31.1"

num_cores=$(grep process /proc/cpuinfo | wc -l)

gnu_url="https://ftp.gnu.org/gnu"

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" > /dev/null 2>&1 && pwd)"
tools_dir="$root_dir/tools"
tools_src_dir="$tools_dir/src"
tools_cross_dir="$tools_dir/cross"

PREFIX="$tools_cross_dir"
PATH="$PREFIX/bin:$PATH"


red="\x1b[0;31m"
green="\x1b[0;32m"
reset="\x1b[0m"


build_binutils() {
	TARGET=$1
	# Does ld exist?
	if [ $(command -v "$TARGET-ld") ]; then
		# Exists. Return
		return
	fi
	# No
	echo -e "\n================ Build binutils ================"
	# 1. Download source
	mkdir -p "$tools_src_dir"
	cd "$tools_src_dir"
	if [ ! -f "binutils-$binutils_ver.tar.xz" ]; then
		echo -n "Downloading ..."
		wget -q -O "binutils-$binutils_ver.tar.xz" \
			"$gnu_url/binutils/binutils-$binutils_ver.tar.xz"
		echo " Done"
	fi
	echo -e "${green}* binutils-$binutils_ver.tar.xz downloaded${reset}"
	# 2. Extract source
	if [ ! -d "binutils-$binutils_ver" ]; then
		echo -n "Extracting ..."
		tar -xf "binutils-$binutils_ver.tar.xz"
		echo " Done"
	fi
	echo -e "${green}* binutils-$binutils_ver extracted${reset}"
	# 3. Build
	if [ -z $(command -v "$TARGET-ld") ]; then
		echo "Building ..."
		rm -rf "binutils-$binutils_ver/build"
		mkdir "binutils-$binutils_ver/build"
		cd "binutils-$binutils_ver/build"
		../configure \
			--target="$TARGET" \
			--prefix="$PREFIX" \
			--with-sysroot \
			--disable-nls \
			--disable-werror > /dev/null
		make -j $num_cores > /dev/null
		make install -j $num_cores > /dev/null
		echo " Done"
	fi
	echo -e "${green}* binutils-$binutils_ver built${reset}"
	echo -e "===================== Done =====================\n"
}


build_gcc() {
	TARGET=$1
	# Does gcc exist?
	if [ $(command -v "$TARGET-gcc") ]; then
		# Exists. Return
		return
	fi
	# No
	echo -e "\n================ Build gcc ================"
	# 1. Download source
	mkdir -p "$tools_src_dir"
	cd "$tools_src_dir"
	if [ ! -f "gcc-$gcc_ver.tar.xz" ]; then
		echo -n "Downloading ..."
		wget -q -O "gcc-$gcc_ver.tar.xz" \
			"$gnu_url/gcc/gcc-$gcc_ver/gcc-$gcc_ver.tar.xz"
		echo " Done"
	fi
	echo -e "${green}* gcc-$gcc_ver.tar.xz downloaded${reset}"
	# 2. Extract source
	if [ ! -d "gcc-$gcc_ver" ]; then
		echo -n "Extracting ..."
		tar -xf "gcc-$gcc_ver.tar.xz"
		echo " Done"
	fi
	echo -e "${green}* gcc-$gcc_ver extracted${reset}"
	# 3. Build
	if [ -z $(command -v "$TARGET-gcc") ]; then
		echo "Building ..."
		rm -rf "gcc-$gcc_ver/build"
		mkdir "gcc-$gcc_ver/build"
		cd "gcc-$gcc_ver/build"
		../configure \
			--target="$TARGET" \
			--prefix="$PREFIX" \
			--disable-nls \
			--enable-languages=c,c++ \
			--without-headers > /dev/null
		make all-gcc -j $num_cores > /dev/null
		CRTSTUFF_T_CFLAGS="$2" make all-target-libgcc -j $num_cores > /dev/null
		make install-gcc -j $num_cores > /dev/null
		make install-target-libgcc -j $num_cores > /dev/null
		echo " Done"
	fi
	echo -e "${green}* gcc-$gcc_ver built${reset}"
	echo -e "===================== Done =====================\n"
}


build_tools() {
	build_binutils $1
	build_gcc $1 $2
}


build_shusboots() {
	# 1. Build toolchain
	build_tools "i686-elf"
	# 2. Build shusboots
	cd "$root_dir"
	make -C shusboots $1 > /dev/null
}


case $# in
	0)
		build_shusboots
		;;
	1)
		case $1 in
			"-h")
				print_usage
				;;
			*)
				build_shusboots $1
				;;
		esac
		;;
	*)
		print_usage
		exit 1
		;;
esac

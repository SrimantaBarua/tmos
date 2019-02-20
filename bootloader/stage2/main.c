// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <log.h>
#include <mem.h>
#include <vesa.h>
#include <term.h>

void main(uint32_t mem_map_base, const struct vbe_info *vbe_info,
		const struct vbe_mode_info *mode_info) {
	union region *regions = 0;
	uint32_t num_regions;

	// Initialize the serial port interface
	serial_init();

	// Initialize memory regions
	num_regions = mem_load_regions(mem_map_base, &regions);

	// Initialize terminal
	term_init(mode_info);

	vlog("Testing display\n");

	// Halt
	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

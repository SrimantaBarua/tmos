// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <mem.h>
#include <log.h>
#include <vesa.h>
#include <term.h>


void main(uint32_t mem_map_base) {
	union region *regions = 0;
	uint32_t num_regions;
	struct vbe_mode_info *mode_info;

	// Initialize the serial port interface
	serial_init();

	// Initialize memory regions
	num_regions = mem_load_regions(mem_map_base, &regions);

	// Initialize VESA subsystem
	if (!(mode_info = vesa_init(1280, 768, 32))) {
		return;
	}

	// Initialize terminal
	term_init(mode_info);
	vlog("Testing display\n");

	// Set new mode
	if (!(mode_info = vesa_set_mode(1024, 768, 32))) {
		return;
	}
	term_init(mode_info);
	vlog("Testing display after resize\n");

	term_reset();
}

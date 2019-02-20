// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <log.h>
#include <mem.h>
#include <vesa.h>
#include <display.h>

void main(uint32_t mem_map_base, const struct vbe_info *vbe_info,
		const struct vbe_mode_info *mode_info) {
	// Initialize the serial port interface
	serial_init();
	// Log that we're starting up
	log(LOG_INFO, "Shu's putting on her Boots..\n");
	union region *regions = 0;
	uint32_t num_regions = mem_load_regions(mem_map_base, &regions);

	struct color bg_color = { .red = 0x00, .green = 0x00, .blue = 0x00  };
	struct color fg_color = { .red = 0xee, .green = 0xee, .blue = 0xee  };
	// Initialize display
	display_init(mode_info);
	// Clear display
	display_clear(bg_color);
	// Write characters
	uint32_t i;
	const char *l0 = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
	const char *l1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	const char *l2 = "abcdefghijklmnopqrstuvwxyz";
	const char *l3 = "0123456789";
	for (i = 0; l0[i]; i++) {
		display_char(l0[i], 2, i * 8 + 8, fg_color);
	}
	for (i = 0; l1[i]; i++) {
		display_char(l1[i], 18, i * 8 + 8, fg_color);
	}
	for (i = 0; l2[i]; i++) {
		display_char(l2[i], 34, i * 8 + 8, fg_color);
	}
	for (i = 0; l3[i]; i++) {
		display_char(l3[i], 50, i * 8 + 8, fg_color);
	}
	display_flush();

	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

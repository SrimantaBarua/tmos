// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <mem.h>
#include <log.h>
#include <vesa.h>
#include <term.h>
#include <part.h>
#include <disk.h>
#include <fs/fs.h>


void main(uint32_t mem_map_base, uint8_t boot_drive) {
	union region *regions = 0;
	uint32_t num_regions, i;
	struct vbe_mode_info *mode_info;
	struct mbr_part *partitions;
	struct fs fs;

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

	// Initialize disk subsystem
	if (disk_init(boot_drive) < 0) {
		return;
	}

	// Get partition info
	partitions = (struct mbr_part*) 0x7dbe;
	for (i = 0; i < 4; i++) {
		if (partitions[i].status == 0x80) {
			break;
		}
	}
	if (i == 4) {
		log(LOG_ERR, "Failed to find bootable partition\n");
		return;
	}

	// Bootable
	log(LOG_INFO, "Bootable partition: %u\n", i);
	vlog("Bootable partition: %u\n", i);
	fs_init(&fs, partitions[i].lba_first, partitions[i].num_sectors);
}

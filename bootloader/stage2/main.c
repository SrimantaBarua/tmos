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
#include <boot_cfg.h>


void main(uint32_t mem_map_base, uint8_t boot_drive) {
	int len;
	char *cfg_buf = (char*) 0x50000;
	union region *regions = 0;
	uint32_t num_regions, i;
	struct vbe_mode_info *mode_info;
	struct mbr_part *partitions;
	struct fs fs;
	const struct boot_cfg *boot_cfg;

	// Initialize the serial port interface
	serial_init();

	// Initialize memory regions
	num_regions = mem_load_regions(mem_map_base, &regions);

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
		log(LOG_ERR, "main: Failed to find bootable partition\n");
		return;
	}

	// Bootable
	log(LOG_INFO, "main: Bootable partition: %u\n", i);
	if (fs_init(&fs, partitions[i].lba_first, partitions[i].num_sectors) < 0) {
		return;
	}
	if (fs.backend && fs.backend->read) {
		if ((len = fs.backend->read(&fs, "/boot.cfg", cfg_buf, 4096)) < 0) {
			return;
		}
		cfg_buf[len] = '\0';
		if (!(boot_cfg = boot_cfg_parse(cfg_buf))) {
			log(LOG_ERR, "main: Failed to parse boot.cfg\n");
		}
		// Initialize display
		if (!(mode_info = vesa_init(boot_cfg->video.width, boot_cfg->video.height, boot_cfg->video.bpp))) {
			log(LOG_ERR, "main: Failed to select requested video mode\n");
		} else {
			term_init(mode_info);
			vlog("Yo, here\n");
		}
	} else {
		log(LOG_ERR, "main: Can't read files with this backend\n");
	}
}

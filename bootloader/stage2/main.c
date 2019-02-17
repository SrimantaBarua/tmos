// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <log.h>
#include <mem.h>


struct ptr {
	uint16_t off;
	uint16_t seg;
} __attribute__ ((packed));

struct vbe_info {
	char       signature[4];
	uint16_t   version;
	struct ptr oem;
	uint32_t   capabilities;
	struct ptr video_modes;
	uint16_t   video_memory;
	uint16_t   software_rev;
	struct ptr vendor;
	struct ptr product_name;
	struct ptr product_rev;
	char       rsvd[222];
	char       oem_data[256];
} __attribute__ ((packed));


void main(struct vbe_info *vbe_info) {
	// Initialize the serial port interface
	serial_init();
	// Log that we're starting up
	log(LOG_INFO, "Shu's putting on her Boots..\n");
	union region *regions = 0;
	uint32_t num_regions = mem_load_regions(0x10000, &regions);

	// Print VESA BIOS information struct
	log(LOG_INFO, "VESA BIOS Information:\n"
		"{\n"
		"  Signature     : %c%c%c%c\n"
		"  Version       : %u\n"
		"  OEM           : 0x%x:0x%x\n"
		"  Capabilities  : 0x%x\n"
		"  Video Modes   : 0x%x:0x%x\n"
		"  Video Memory  : %u\n"
		"  Software Rev. : %u\n"
		"  Vendor        : 0x%x:0x%x\n"
		"  Product Name  : 0x%x:0x%x\n"
		"  Product Rev.  : 0x%x:0x%x\n"
		"}\n",
		vbe_info->signature[0], vbe_info->signature[1], vbe_info->signature[2],
		vbe_info->signature[3], vbe_info->version, vbe_info->oem.seg, vbe_info->oem.off,
		vbe_info->capabilities, vbe_info->video_modes.seg, vbe_info->video_modes.off,
		vbe_info->video_memory, vbe_info->software_rev, vbe_info->vendor.seg,
		vbe_info->vendor.off, vbe_info->product_name.seg, vbe_info->product_name.off,
		vbe_info->product_rev.seg, vbe_info->product_rev.off);
	volatile uint32_t *ptr = 0xb8000;
	*ptr = 0x2f4b2f4f;
	PANIC("Test panic: %d\n", 3);
	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

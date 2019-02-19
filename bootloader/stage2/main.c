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


struct vbe_mode_info {
	uint16_t attributes;
	uint8_t  window_a;
	uint8_t  window_b;
	uint16_t granularity;
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;
	uint16_t pitch;
	uint16_t width;
	uint16_t height;
	uint8_t  w_char;
	uint8_t  y_char;
	uint8_t  planes;
	uint8_t  bpp;
	uint8_t  banks;
	uint8_t  model;
	uint8_t  bank_size;
	uint8_t  image_pages;
	uint8_t  reserved0;
	uint8_t  red_mask_size;
	uint8_t  red_mask_pos;
	uint8_t  green_mask_size;
	uint8_t  green_mask_pos;
	uint8_t  blue_mask_size;
	uint8_t  blue_mask_pos;
	uint8_t  reserved_mask_size;
	uint8_t  reserved_mask_pos;
	uint8_t  direct_color_attributes;
	uint32_t framebuffer;
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;
	uint8_t  reserved1[206];
};


void main(uint32_t mem_map_base, struct vbe_info *vbe_info, struct vbe_mode_info *mode_info) {
	// Initialize the serial port interface
	serial_init();
	// Log that we're starting up
	log(LOG_INFO, "Shu's putting on her Boots..\n");
	union region *regions = 0;
	uint32_t num_regions = mem_load_regions(mem_map_base, &regions);

	// Print VESA BIOS information struct
	log(LOG_INFO, "Ptr:  0x%x\n", vbe_info);
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

	log(LOG_INFO, "Video modes:\n");
	while (1) {
		if (mode_info->height == 0 && mode_info->width == 0) {
			break;
		}
		if (mode_info->model != 6) {
			mode_info++;
			continue;
		}
		__log_without_typ("{\n"
			"  height     : %u\n"
			"  width      : %u\n"
			"  pitch      : %u\n"
			"  bpp        : %u\n"
			"  model      : %u\n"
			"  red_mask   : 0x%x\n"
			"  red_pos    : 0x%x\n"
			"  green_mask : 0x%x\n"
			"  green_pos  : 0x%x\n"
			"  blue_mask  : 0x%x\n"
			"  blue_pos   : 0x%x\n"
			"  rsvd_mask  : 0x%x\n"
			"  rsvd_pos   : 0x%x\n"
			"  framebuf   : 0x%x\n"
			"  dircolatt  : 0x%x\n"
			"}\n",
			mode_info->height, mode_info->width, mode_info->pitch, mode_info->bpp,
			mode_info->model, mode_info->red_mask_size, mode_info->red_mask_pos,
			mode_info->green_mask_size, mode_info->green_mask_pos,
			mode_info->blue_mask_size, mode_info->blue_mask_pos,
			mode_info->reserved_mask_size, mode_info->reserved_mask_pos,
			mode_info->framebuffer, mode_info->direct_color_attributes);
		mode_info++;
	}

	volatile uint32_t *ptr = 0xb8000;
	*ptr = 0x2f4b2f4f;
	PANIC("Test panic: %d\n", 3);
	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

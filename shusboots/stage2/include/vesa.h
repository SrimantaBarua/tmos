// (C) 2019 Srimanta Barua
//
// The stage 1.5 of the bootloader reads in VESA BIOS information from the BIOS, selects and sets
// a display mode, and sends pointers to structures for VESA BIOS information and display mode
// information to our C code.
//
// The selected mode is guaranteed to be a 32-bit direct-color RGB mode with framebuffer support.


#pragma once

#include <real.h>

// VESA BIOS information
struct vbe_info {
	char            signature[4];
	uint16_t        version;
	struct real_ptr oem;
	uint32_t        capabilities;
	struct real_ptr video_modes;
	uint16_t        video_memory;
	uint16_t        software_rev;
	struct real_ptr vendor;
	struct real_ptr product_name;
	struct real_ptr product_rev;
	char            rsvd[222];
	char            oem_data[256];
} __attribute__ ((packed));


// Display mode information
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


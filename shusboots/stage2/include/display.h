// (C) 2019 Srimanta Barua
//
// Interface for displaying things on screen

#pragma once

#include <stdint.h>
#include <vesa.h>

// Display pixel
struct color {
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	uint8_t __rsvd;
} __attribute__ ((packed));

// Initialize display
void display_init(const struct vbe_mode_info *vbe_mode_info);

// Put a pixel at given (Y, X) coordinate
void display_putpixel(uint16_t y, uint16_t x, struct color color);

// Clear screen with given color
void display_clear(struct color color);

// Draw a rectangle of given color
void display_rect(uint16_t ytl, uint16_t xtl, uint16_t ybr, uint16_t xbr, struct color color);

// Draw a 16x8 character of given color
void display_char(char c, uint16_t ytl, uint16_t xtl, struct color color);

// Flush memory buffer to display buffer
void display_flush();

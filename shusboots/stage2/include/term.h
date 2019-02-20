// (C) 2019 Srimanta Barua
//
// A higher-level interface for writing text to the display

#pragma once

#include <vesa.h>
#include <stdint.h>
#include <display.h>

// Initialize terminal
void term_init(const struct vbe_mode_info *vbe_mode_info);

// Write string to terminal
void term_write_str(const char *str);

// Change color of text or background on terminal
void term_set_fg_color(struct color fg);

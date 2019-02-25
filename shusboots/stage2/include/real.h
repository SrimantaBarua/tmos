// (C) 2019 Srimanta Barua
//
// Definitions for artifacts from real mode, like segment:offset pointers


#pragma once

#include <stdint.h>

// Real mode pointer, which is represented as segment:offset
struct real_ptr {
	uint16_t off;
	uint16_t seg;
};

// Get linear pointer corresponding to real mode pointer
#define real_ptr_to_linear(rp) ((((uint32_t) (rp).seg) << 4) | (uint32_t) (rp).off)

// Run a function in real mode
uint32_t real_call(void (*func) (void*), uint32_t arg_count, ...);

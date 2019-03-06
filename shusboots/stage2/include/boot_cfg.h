// (C) 2019 Srimanta Barua
//
// Parse bootloader config file

#pragma once


#include <stdint.h>


// Parsed boot.cfg structure
struct boot_cfg {
	struct {
		uint16_t width;
		uint16_t height;
		uint8_t bpp;
	} video;
	struct {
		const char *path;
		const char *root;
	} kernel;
};


// Parse bootloader config (null-terminated string). Returns pointer to parsed structure if valid,
// else return NULL
const struct boot_cfg* boot_cfg_parse(char *cfg);

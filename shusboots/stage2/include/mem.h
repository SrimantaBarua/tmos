// (C) 2018 Srimanta Barua
//
// The BIOS gives us a memory map stored in a not-very-efficient manner. Plus it could have
// overlaps, be unsorted, and is probably not continuous. This module sorts and cleans the BIOS
// memory map, then translates it to a more efficient internal format

#pragma once

#include <stdint.h>

// Our memory region format
union region {
	struct {
		uint64_t _rsvd0 : 8;
		uint64_t start  : 44;
		uint64_t _rsvd1 : 10;
		uint64_t type   : 2;
	} val;
	uint64_t raw;

} __attribute__((packed));

// Load an array of memory regions from addr of BIOS memory map, and return number of regions
uint32_t mem_load_regions(uint32_t addr, union region **regions);

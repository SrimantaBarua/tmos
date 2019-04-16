// (C) 2018 Srimanta Barua
//
// The BIOS gives us a memory map stored in a not-very-efficient manner. Plus it could have
// overlaps, be unsorted, and is probably not continuous. This module sorts and cleans the BIOS
// memory map, then translates it to a more efficient internal format
//
// The structure of a BIOS memory map region is described in stage1/boot_mbr.asm . This uses
// 24 bytes per region. Plus, it's not continuous. There was a discussion on the OS-Dev Wiki forums
// which I can't find now, which uses a 64-bit value to store a memory region. It stores the
// page-aligned start address of the region. And since physical addresses are of a max of 56 bits,
// this means that we have 20 bits to store various flags.
//
// This also means that there is not 'end' indicator for a region. The end of the region is just
// the start address of the next region. This ensures that the memory map is continuous. The end of
// the region with the highest start address is 0x00ff_ffff_ffff_ffff.
//
// The regions are stored in decreasing order of start address. This way, the region with a start
// address of 0 denotes the end of the map, so that we don't need any additional metadata to store
// the length of the map.

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

// Type of memory region
#define REGION_TYPE_AVAIL        0
#define REGION_TYPE_RSVD         1
#define REGION_TYPE_ACPI_RECLAIM 2
#define REGION_TYPE_ACPI_NVS     3

// Load an array of memory regions from addr of BIOS memory map, and return number of regions
uint32_t mem_load_regions(uint32_t addr, union region **regions);

// Create a new region of the given type and start address. Start address is page aligned
union region region_new(uint64_t start_addr, uint8_t type);

// Get start address of region
#define REGION_START(r) ((r).val.start << 12)
#define REGION_TYPE(r) ((r).val.type)
#define REGION_SET_START(r, addr) {                          \
	(r).val.start = ((addr) & 0x00ffffffffffffff) >> 12; \
}
#define REGION_SET_TYPE(r, type) {  \
	(r).val.type = (type) & 3;  \
}

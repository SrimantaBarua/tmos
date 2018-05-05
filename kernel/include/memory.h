// (C) 2018 Srimanta Barua
// Parse BIOS/EFI/other memory maps into our own internal memory map format
//
// This memory map format is inspired from a thread on the OS-Dev Wiki forums, which I can't find
// now, which uses a 64-bit value to store a memory region. It stores the page-aligned start
// address of the region. And since physical addresses are of a max of 56 bits, this means that
// we have 20 bits to store various flags.
//
// This also means that there is not 'end' indicator for a region. The end of the region is just
// the start address of the next region. This ensures that the memory map is continuous. The end of
// the region with the highest start address is 0x00ff_ffff_ffff_ffff.
//
// The regions are stored in decreasing order of start address. This way, the region with a start
// address of 0 denotes the end of the map, so that we don't need any additional metadata to store
// the length of the map.


#pragma once

#include <system.h>

// A memory map region entry
typedef uint64_t region_t;

// Type of memory region
#define REGION_TYPE_AVAIL        0
#define REGION_TYPE_RSVD         1
#define REGION_TYPE_ACPI_RECLAIM 2
#define REGION_TYPE_ACPI_NVS     3
#define __REGION_TYPE_MASK       3

// Load an array of memory regions from multiboot2 memory map, and return number of regions
uint32_t mem_load_mb2_mmap(region_t **regions);

// Create a new region of the given type and start address. Start address is page aligned
region_t region_new(uint64_t start_addr, uint8_t type);

// Get start address of region
#define REGION_START(r) ((r) & PADDR_ALGN_MASK)

// Get type of region
#define REGION_TYPE(r) ((r) & __REGION_TYPE_MASK)

// Set the start address of the region
#define REGION_SET_START(r, addr) {                                \
	(r) = ((r) & __REGION_TYPE_MASK) | (addr & PADDR_ALGN_MASK); \
}

// Set type of region
#define REGION_SET_TYPE(r, type) {                                       \
	(r) = ((r) & PADDR_ALGN_MASK) | (type & __REGION_TYPE_MASK); \
}

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
#define REGION_TYPE_NONE         0
#define REGION_TYPE_AVAIL        1
#define REGION_TYPE_MULTIBOOT2   2
#define REGION_TYPE_KERNEL       3
#define REGION_TYPE_ACPI_RECLAIM 4
#define REGION_TYPE_ACPI_NVS     5
#define REGION_TYPE_RSVD         6
#define __REGION_TYPE_MASK       7

// Region flags
#define __REGION_FLAG_SHIFT  3
#define REGION_FLAG_MANAGED  (1 << __REGION_FLAG_SHIFT)

// Get start address of region
#define REGION_START(r) ((r) & PADDR_ALGN_MASK)

// Get type of region
#define REGION_TYPE(r) ((r) & __REGION_TYPE_MASK)

// Return a new region with given start address and type
#define REGION_NEW(addr, typ) (((uint64_t) addr & PADDR_ALGN_MASK) | ((uint64_t) typ & __REGION_TYPE_MASK))

// Set the start address of the region
#define REGION_SET_START(r, addr) {                                \
	(r) = ((r) & __REGION_TYPE_MASK) | (addr & PADDR_ALGN_MASK); \
}

// Set type of region
#define REGION_SET_TYPE(r, type) {                                       \
	(r) = ((r) & PADDR_ALGN_MASK) | (type & __REGION_TYPE_MASK); \
}

// Set a region as managed
#define REGION_SET_MANAGED(r) { \
	(r) |= REGION_FLAG_MANAGED; \
}

// Set a region as unmanaged
#define REGION_SET_UNMANAGED(r) { \
	(r) &= ~REGION_FLAG_MANAGED; \
}


// A memory map made up of an array of our regions, and the number of regions
// Hopefully we won't have more than 128 regions...

#define MMAP_MAX_NUM_ENTRIES 128

struct mmap {
	region_t r[MMAP_MAX_NUM_ENTRIES];
};

// Load an array of memory regions from multiboot2 memory map, and return number of regions
void mem_load_mb2_mmap(struct mmap *map);

// Insert a region into a memory map
void mmap_insert_region(struct mmap *map, uint64_t start_addr, uint64_t end, uint32_t type);

// Split a memory map at the given address. Splits any regions covering the address into two new
// regions.
void mmap_split_at(struct mmap *map, uint64_t addr);

// Print the memory map
void mmap_print(const struct mmap *map);


// Interface for a physical memory manager
struct pmmgr {
	// Initialize the physical memory manager to handle the given regions
	// The first element in the regions array denotes the last address, and is not to be
	// managed by the physical memory manager. However, num_regions includes this region
	// Also denote the limits of memory that we can freely use for the fast one-frame alloc
	// free functions
	void (*init) (region_t *regions, uint32_t num_regions, paddr_t fast_start, paddr_t fast_end);
	// Allocate one frame (for fast 1-frame allocators)
	paddr_t (*alloc) (void);
	// Free one frame (for fast 1-frame allocators)
	void (*free) (paddr_t addr);
	// Allocate with given requirements (for special allocators)
	// between `above` and `below`, aligned to (1 << `align`) page size, `num` frames
	paddr_t (*spl_alloc) (paddr_t above, paddr_t below, uint32_t align, uint32_t num);
	// Free given range of pages (for special allocators)
	void (*spl_free) (paddr_t addr, uint32_t num);
	// Optional callback for mapping the bitmap into a new address space
	// eg. when setting up paging on x86/x86_64
	void (*remap_cb) ();
};

// Known physical memory managers
extern struct pmmgr BM_SPL_PMMGR, BM_PMMGR;


// Initialize the memory management subsystem with the given underlying physical memory manager
// Also provide an optional callback for remapping in case required
void mem_init(struct pmmgr *pmmgr, void (*remap_cb) (void));

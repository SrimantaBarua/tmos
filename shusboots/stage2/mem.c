// (C) 2018 Srimanta Barua

#include <log.h>
#include <mem.h>

// The memory map structure that the BIOS gives us
struct bios_region {
	uint64_t base;
	uint64_t len;
	uint32_t type;
	uint32_t acpi;
};

// Type of a bios region
#define TYPE(r) (((r.type) == 0 || (r.type) > 4) ? 2 : (r.type))

// Swap two bios memory map regions
static void _swap_bios_region(struct bios_region *a, struct bios_region *b) {
	struct bios_region c = *a;
	*a = *b;
	*b = c;
}

// Sort the bios memory map array in increasing order of base address. If base address is the same
// then sort them in increasing order of length
static void _sort_map(struct bios_region *mmap, uint32_t len) {
	uint32_t i, j, min_idx;
	for (i = 0; i < len - 1; i++) {
		min_idx = i;
		for (j = i + 1; j < len; j++) {
			if (mmap[j].base < mmap[min_idx].base) {
				min_idx = j;
			}
		}
		if (mmap[i].base == mmap[min_idx].base) {
			if (mmap[i].len <= mmap[min_idx].len) {
				continue;
			}
		}
		_swap_bios_region (&mmap[i], &mmap[min_idx]);
	}
}

// Load an array of memory regions from addr of BIOS memory map, and return number of regions
uint32_t mem_load_regions(uint32_t addr, union region **regions) {
	uint32_t i;
	uint32_t bios_mmap_len = *((uint64_t*) addr);
	struct bios_region *bios_mmap = (struct bios_region*) (addr + 8);
	// Sort the map
	_sort_map (bios_mmap, bios_mmap_len);
	// Solve overlap and merge regions

	log (LOG_INFO, "BIOS MMAP REGIONS (%u): [\n", bios_mmap_len);
	for (i = 0; i < bios_mmap_len; i++) {
		__log_without_typ ("\t{ B: 0x%08llx | L: 0x%08llx | T: %d | A: %d }\n",
				bios_mmap[i].base, bios_mmap[i].len,
				bios_mmap[i].type, bios_mmap[i].acpi);
	}
	__log_without_typ ("]\n");

	return bios_mmap_len;
}

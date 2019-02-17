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

#define TYPE(m) (((m).type == 0 || (m).type > 4) ? 2 : (m).type)
#define END(m) ((m).base + (m).len)
#define PAGE_ALGN_UP(x) (((x) + 4095) & !4095)
#define PAGE_ALGN_DWN(x) ((x) & !4095)

// Swap two bios memory map regions
static void _swap_bios_region(struct bios_region *a, struct bios_region *b) {
	struct bios_region c = *a;
	*a = *b;
	*b = c;
}

// Sort the bios memory map array in decreasing order of end address. If end address is the same
// then sort them in decreasing order of base_address
static void _sort_map(struct bios_region *mmap, uint32_t len) {
	uint32_t i, j, max_idx;
	for (i = 0; i < len - 1; i++) {
		max_idx = i;
		for (j = i + 1; j < len; j++) {
			if (END(mmap[j]) > END(mmap[max_idx])) {
				max_idx = j;
			}
		}
		if (END(mmap[i]) == END(mmap[max_idx])) {
			if (mmap[i].base >= mmap[max_idx].base) {
				continue;
			}
		}
		_swap_bios_region(&mmap[i], &mmap[max_idx]);
	}
}

// Translate the BIOS memory map into regions
static uint32_t _translate(struct bios_region *mmap, uint32_t len, union region **regions) {
	if (!regions || len == 0) {
		return 0;
	}
	uint32_t reglen = 0, i;
	uint64_t last_start = (uint64_t) 1 << 56, last_end;
	union region *tmp_region = (union region *) &mmap[len];
	for (i = 0; i < len; i++) {
		if (PAGE_ALGN_UP(END(mmap[i])) < last_start) {
			if (reglen > 0 && REGION_TYPE(tmp_region[reglen - 1]) == REGION_TYPE_RSVD) {
				REGION_SET_START(tmp_region[reglen - 1], END(mmap[i]));
				last_start = REGION_START(tmp_region[reglen - 1]);
			} else {
				tmp_region[reglen] = region_new(END(mmap[i]), REGION_TYPE_RSVD);
				last_start = REGION_START(tmp_region[reglen]);
				reglen++;
			}
		}
		if (reglen > 0 && REGION_TYPE(tmp_region[reglen - 1]) == TYPE(mmap[i]) - 1) {
			if (mmap[i].base >= last_start) {
				continue;
			}
			REGION_SET_START(tmp_region[reglen - 1], mmap[i].base);
			last_start = REGION_START(tmp_region[reglen -  1]);
			continue;
		}
		if (PAGE_ALGN_DWN(END(mmap[i])) > last_start) {
			// Overlap
			ASSERT(reglen != 0); // We don't handle that memory is upto LAST_START
			if (reglen > 1) {
				last_end = REGION_START(tmp_region[reglen - 2]);
			} else {
				last_end = (uint64_t) 1 << 56;
			}
			switch (REGION_TYPE(tmp_region[reglen - 1])) {
			case 0:
				if (mmap[i].base <= last_start) {
					if (END(mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
						break;
					}
					REGION_SET_START(tmp_region[reglen - 1], END(mmap[i]));
					tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				if (END(mmap[i]) == last_end) {
					tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 1]), 0);
					tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 2]), 0);
				tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
				REGION_SET_START(tmp_region[reglen - 2], END(mmap[i]));
				reglen++;
				break;
			case 1:
				if (mmap[i].base < last_start) {
					tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
				}
				break;
			case 2:
				if (TYPE(mmap[i]) == 1) {
					if (mmap[i].base < last_start) {
						tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
						reglen++;
					}
					break;
				}
				if (mmap[i].base <= last_start) {
					if (END(mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
						break;
					}
					REGION_SET_START(tmp_region[reglen - 1], END(mmap[i]));
					tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				if (END(mmap[i]) == last_end) {
					tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 1]), 2);
					tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 2]), 2);
				tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
				REGION_SET_START(tmp_region[reglen - 2], END(mmap[i]));
				reglen++;
				break;
			case 3:
				if (TYPE(mmap[i]) != 2) {
					if (mmap[i].base < last_start) {
						tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
						reglen++;
					}
					break;
				}
				if (mmap[i].base <= last_start) {
					if (END(mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
						break;
					}
					REGION_SET_START(tmp_region[reglen - 1], END(mmap[i]));
					tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				if (END(mmap[i]) == last_end) {
					tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 1]), 3);
					tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new(REGION_START(tmp_region[reglen - 2]), 3);
				tmp_region[reglen - 1] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
				REGION_SET_START(tmp_region[reglen - 2], END(mmap[i]));
				reglen++;
			}
			continue;
		}
		tmp_region[reglen] = region_new(mmap[i].base, TYPE(mmap[i]) - 1);
		last_start = REGION_START(tmp_region[reglen]);
		reglen++;
	}
	log(LOG_INFO, "REGIONS (%u): [\n", reglen);
	for (i = 0; i < reglen; i++) {
		__log_without_typ("\t{ B: 0x%08llx | T: %d }\n",
				REGION_START(tmp_region[i]),
				REGION_TYPE(tmp_region[i]));
	}
	__log_without_typ("]\n");
	return reglen;
}

// Load an array of memory regions from addr of BIOS memory map, and return number of regions
uint32_t mem_load_regions(uint32_t addr, union region **regions) {
	uint32_t i;
	uint32_t bios_mmap_len = *((uint64_t*) addr);
	struct bios_region *bios_mmap = (struct bios_region*) (addr + 8);
	// Sort the map
	_sort_map(bios_mmap, bios_mmap_len);

	log(LOG_INFO, "BIOS MMAP REGIONS (%u): [\n", bios_mmap_len);
	for (i = 0; i < bios_mmap_len; i++) {
		__log_without_typ("\t{ B: 0x%08llx | L: 0x%08llx | T: %d | A: %d }\n",
				bios_mmap[i].base, bios_mmap[i].len,
				bios_mmap[i].type, bios_mmap[i].acpi);
	}
	__log_without_typ("]\n");

	// Translate the map into regions
	return _translate(bios_mmap, bios_mmap_len, regions);
}

// Create a new region of the given type and start address. Start address is page aligned
union region region_new(uint64_t start_addr, uint8_t type) {
	union region reg = { .raw = 0 };
	REGION_SET_START(reg, start_addr);
	REGION_SET_TYPE(reg, type);
	return reg;
}

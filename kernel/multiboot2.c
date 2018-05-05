// (C) 2018 Srimanta Barua

#include <stddef.h>
#include <multiboot2.h>
#include <klog.h>

// Global pointer to multiboot table. Doesn't not need locking because it is only set once
const struct mb2_table *MB2TAB = NULL;

// Load multiboot2 table from address after checking if valid. -1 on error, 0 on success
int mb2_table_load(vaddr_t addr) {
	const struct mb2_tag *last_tag;
	const struct mb2_table *table;
	table = (const struct mb2_table*) addr;
	if (MB2TAB) {
		return -1;
	}
	if (!table->size || table->_rsvd) {
		return -1;
	}
	// Check if last tag is valid
	last_tag = (const struct mb2_tag*) (((void*) table) + table->size - 8);
	if (last_tag->type != 0 || last_tag->size != 8) {
		return -1;
	}
	MB2TAB = table;
	return 0;
}

// Get a pointer to a multiboot2 tag of the given type
const struct mb2_tag* mb2_get_tag(uint32_t type) {
	const struct mb2_tag *ret;
	if (!MB2TAB) {
		return NULL;
	}
	ret = (const struct mb2_tag*) (((void*) MB2TAB) + sizeof (struct mb2_table));
	while (ret && ret->type) {
		if (ret->type == type) {
			return ret;
		}
		ret = (const struct mb2_tag*) (((void*) ret) + ((ret->size + 7) & ~7));
	}
	return NULL;
}

#include <memory.h>

#define MB2_MREG_TYPE(m) (((m).type == 0 || (m).type > 4) ? 2 : (m).type)
#define MB2_MREG_END(m) ((m).start + (m).len)

// Sort the MB2 memory map array in decreasing order of end address. If end address is the same
// then sort them in decreasing order of start address
static void _sort_mb2_map(struct mb2_mmap_region *mmap, uint32_t len) {
	uint32_t i, j, max_idx;
	struct mb2_mmap_region tmp;
	for (i = 0; i < len - 1; i++) {
		max_idx = i;
		for (j = i + 1; j < len; j++) {
			if (MB2_MREG_END (mmap[j]) > MB2_MREG_END (mmap[max_idx])) {
				max_idx = j;
			}
		}
		if (MB2_MREG_END (mmap[i]) == MB2_MREG_END (mmap[max_idx])) {
			if (mmap[i].start >= mmap[max_idx].start) {
				continue;
			}
		}
		tmp = mmap[i];
		mmap[i] = mmap[max_idx];
		mmap[max_idx] = tmp;
	}
}

// Translate the MB2 memory map into regions
// TODO: Go through this again thoroughly and review. Especially the parts about page alignment
static uint32_t _mb2_mmap_translate(struct mb2_mmap_region *mmap, uint32_t len, region_t **regions) {
	if (!regions || len == 0) {
		return 0;
	}
	uint32_t reglen = 0, i;
	uint64_t last_start = PADDR_MASK + 1, last_end;
	region_t *tmp_region = (region_t *) &mmap[len];
	for (i = 0; i < len; i++) {
		if (PAGE_ALGN_UP (MB2_MREG_END (mmap[i])) < last_start) {
			if (reglen > 0 && REGION_TYPE (tmp_region[reglen - 1]) == REGION_TYPE_RSVD) {
				REGION_SET_START (tmp_region[reglen - 1], MB2_MREG_END (mmap[i]));
				last_start = REGION_START (tmp_region[reglen - 1]);
			} else {
				tmp_region[reglen] = region_new (MB2_MREG_END (mmap[i]), REGION_TYPE_RSVD);
				last_start = REGION_START (tmp_region[reglen]);
				reglen++;
			}
		}
		if (reglen > 0 && REGION_TYPE (tmp_region[reglen - 1]) == MB2_MREG_TYPE (mmap[i]) - 1) {
			if (mmap[i].start >= last_start) {
				continue;
			}
			REGION_SET_START (tmp_region[reglen - 1], mmap[i].start);
			last_start = REGION_START (tmp_region[reglen -  1]);
			continue;
		}
		if (PAGE_ALGN_DOWN (MB2_MREG_END (mmap[i])) > last_start) {
			// Overlap
			ASSERT (reglen != 0); // We don't handle that memory is upto LAST_START
			if (reglen > 1) {
				last_end = REGION_START (tmp_region[reglen - 2]);
			} else {
				last_end = PADDR_MASK + 1;
			}
			switch (REGION_TYPE (tmp_region[reglen - 1])) {
			case 0:
				if (mmap[i].start <= last_start) {
					if (MB2_MREG_END (mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
						break;
					}
					REGION_SET_START (tmp_region[reglen - 1], MB2_MREG_END (mmap[i]));
					tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				if (MB2_MREG_END (mmap[i]) == last_end) {
					tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 1]), 0);
					tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 2]), 0);
				tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
				REGION_SET_START (tmp_region[reglen - 2], MB2_MREG_END (mmap[i]));
				reglen++;
				break;
			case 1:
				if (mmap[i].start < last_start) {
					tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
				}
				break;
			case 2:
				if (MB2_MREG_TYPE (mmap[i]) == 1) {
					if (mmap[i].start < last_start) {
						tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
						reglen++;
					}
					break;
				}
				if (mmap[i].start <= last_start) {
					if (MB2_MREG_END (mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
						break;
					}
					REGION_SET_START (tmp_region[reglen - 1], MB2_MREG_END (mmap[i]));
					tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				if (MB2_MREG_END (mmap[i]) == last_end) {
					tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 1]), 2);
					tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 2]), 2);
				tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
				REGION_SET_START (tmp_region[reglen - 2], MB2_MREG_END (mmap[i]));
				reglen++;
				break;
			case 3:
				if (MB2_MREG_TYPE (mmap[i]) != 2) {
					if (mmap[i].start < last_start) {
						tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
						reglen++;
					}
					break;
				}
				if (mmap[i].start <= last_start) {
					if (MB2_MREG_END (mmap[i]) == last_end) {
						tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
						break;
					}
					REGION_SET_START (tmp_region[reglen - 1], MB2_MREG_END (mmap[i]));
					tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				if (MB2_MREG_END (mmap[i]) == last_end) {
					tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 1]), 3);
					tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
					reglen++;
					break;
				}
				reglen++;
				tmp_region[reglen] = region_new (REGION_START (tmp_region[reglen - 2]), 3);
				tmp_region[reglen - 1] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
				REGION_SET_START (tmp_region[reglen - 2], MB2_MREG_END (mmap[i]));
				reglen++;
			}
			continue;
		}
		tmp_region[reglen] = region_new (mmap[i].start, MB2_MREG_TYPE (mmap[i]) - 1);
		last_start = REGION_START (tmp_region[reglen]);
		reglen++;
	}
	klog ("REGIONS (%u): [\n", reglen);
	for (i = 0; i < reglen; i++) {
		klog ("\t{ B: 0x%08llx | T: %d }\n", REGION_START (tmp_region[i]), REGION_TYPE (tmp_region[i]));
	}
	klog ("]\n");
	return reglen;
}

// Load an array of memory regions from addr of MB2 memory map, and return number of regions
uint32_t mem_load_mb2_mmap(region_t **regions) {
	const struct mb2_tag_mmap *tag;
	struct mb2_mmap_region *mb2_mmap;
	uint32_t mb2_mmap_len, i;

	if (!(tag = (const struct mb2_tag_mmap*) mb2_get_tag (MB2_TAG_TYPE_MMAP))) {
		PANIC ("BIOS memory map tag not found\n");
	}
	mb2_mmap_len = (tag->size - sizeof (struct mb2_tag_mmap)) / tag->entsz;
	mb2_mmap = (struct mb2_mmap_region*) (((void*) tag) + sizeof (struct mb2_tag_mmap));

	// Sort the map
	_sort_mb2_map (mb2_mmap, mb2_mmap_len);

	klog ("MULTIBOOT2 MMAP REGIONS (%u): [\n", mb2_mmap_len);
	for (i = 0; i < mb2_mmap_len; i++) {
		klog ("\t{ B: 0x%08llx | L: 0x%08llx | T: %d | A: %d }\n",
			mb2_mmap[i].start, mb2_mmap[i].len,
			mb2_mmap[i].type, mb2_mmap[i]._rsvd);
	}
	klog ("]\n");

	// Translate the map into regions
	return _mb2_mmap_translate (mb2_mmap, mb2_mmap_len, regions);
}

// Create a new region of the given type and start address. Start address is page aligned
region_t region_new(uint64_t start_addr, uint8_t type) {
	return (start_addr & PADDR_ALGN_MASK) | ((uint64_t) type & __REGION_TYPE_MASK);
}


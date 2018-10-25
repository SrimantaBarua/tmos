// (C) 2018 Srimanta Barua

#include <stddef.h>
#include <shuos/multiboot2.h>
#include <shuos/klog.h>

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
	ret = (const struct mb2_tag*) (((void*) MB2TAB) + sizeof(struct mb2_table));
	while (ret && ret->type) {
		if (ret->type == type) {
			return ret;
		}
		ret = (const struct mb2_tag*) (((void*) ret) + ((ret->size + 7) & ~7));
	}
	return NULL;
}

#include <shuos/memory.h>

#define MB2_MREG_TYPE(m) (((m).type == 0 || (m).type > 4) ? 2 : (m).type)
#define MB2_MREG_END(m) ((m).start + (m).len)

// Convert multiboot2 memory map type to our memory map type
static uint32_t _cvt_mb2_mmap_typ(uint32_t type) {
	switch (type) {
	case 1 : return REGION_TYPE_AVAIL;
	case 2 : return REGION_TYPE_RSVD;
	case 3 : return REGION_TYPE_ACPI_RECLAIM;
	case 4 : return REGION_TYPE_ACPI_NVS;
	}
}

// Fill our memory map from multiboot2 memory map
static void _fill_mmap(struct mb2_mmap_region *mb2, uint32_t mb2_len, struct mmap *regmap) {
	uint32_t i, type;
	uint64_t start, end;
	for (i = 0; i < mb2_len; i++) {
		type = _cvt_mb2_mmap_typ(MB2_MREG_TYPE(mb2[i]));
		if (type == REGION_TYPE_AVAIL) {
			start = PAGE_ALGN_UP(mb2[i].start);
			end = PAGE_ALGN_DOWN(MB2_MREG_END(mb2[i]));
		} else {
			start = PAGE_ALGN_DOWN(mb2[i].start);
			end = PAGE_ALGN_UP(MB2_MREG_END(mb2[i]));
		}
		mmap_insert_region(regmap, start, end, type);
	}
}

// Load an array of memory regions from addr of MB2 memory map, and return number of regions
void mem_load_mb2_mmap(struct mmap *regmap) {
	const struct mb2_tag_mmap *tag;
	struct mb2_mmap_region *mb2_mmap;
	uint32_t mb2_mmap_len, i;
	uint64_t mb2tab_addr = (uint64_t) MB2TAB;

	ASSERT(regmap);

	if (!(tag = (const struct mb2_tag_mmap*) mb2_get_tag(MB2_TAG_TYPE_MMAP))) {
		PANIC("BIOS memory map tag not found\n");
	}
	mb2_mmap_len = (tag->size - sizeof(struct mb2_tag_mmap)) / tag->entsz;
	mb2_mmap = (struct mb2_mmap_region*) (((void*) tag) + sizeof(struct mb2_tag_mmap));

	klog("MULTIBOOT2 MMAP REGIONS (%u): [\n", mb2_mmap_len);
	for (i = 0; i < mb2_mmap_len; i++) {
		klog("\t{ B: 0x%08llx | L: 0x%08llx | T: %d | A: %d }\n",
			mb2_mmap[i].start, mb2_mmap[i].len,
			mb2_mmap[i].type, mb2_mmap[i]._rsvd);
	}
	klog("]\n");

	// Initialize the memory map with default value
	regmap->r[0] = 0 | REGION_TYPE_NONE;

	// Fill the mmap with mb2_mmap
	_fill_mmap(mb2_mmap, mb2_mmap_len, regmap);

	// Mark region for multiboot2 table
	mmap_insert_region(regmap, PAGE_ALGN_DOWN(mb2tab_addr),
			PAGE_ALGN_UP(mb2tab_addr + MB2TAB->size), REGION_TYPE_MULTIBOOT2);
}

// (C) 2018 Srimanta Barua

#include <stddef.h>
#include <multiboot2.h>

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

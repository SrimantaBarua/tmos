// (C) 2018 Srimanta Barua

#include <memory.h>
#include <klog.h>
#include <stdbool.h>

// Find indices for inserting region
static void _find_idx(struct mmap *map, uint64_t start, uint64_t end, uint64_t *i, uint64_t *j) {
	uint64_t _i, _j;
	for (_i = 0; _i < MMAP_MAX_NUM_ENTRIES; _i++) {
		// Find region whose start is <= end
		if (REGION_START (map->r[_i]) > end) {
			continue;
		}
		// Found
		for (_j = _i; _i < MMAP_MAX_NUM_ENTRIES; _j++) {
			// Find region whose start is <= start
			if (REGION_START (map->r[_j]) > start) {
				continue;
			}
			// Found
			*i = _i;
			*j = _j;
			return;
		}
		break;
	}
	// SHOULD NEVER HAPPEN
	PANIC ("unreachable");
}

// Move the map right. Panic if no space. Basically, map[idx..] -> map[(idx + amt)..]
static void _move_right(struct mmap *map, uint64_t idx, uint64_t amt) {
	uint64_t len = 0, i;
	for (i = idx; REGION_START (map->r[i]); i++);
	if (i + amt >= MMAP_MAX_NUM_ENTRIES) {
		PANIC ("No space in memory map");
	}
	len = i + 1 - idx;
	i = idx + len + amt;
	while (len--) {
		map->r[i - 1] = map->r[i - amt - 1];
		i--;
	}
}

// Move the map left. Panics if amt > idx. Basically, map[idx..] -> map[(idx - amt)...]
static void _move_left(struct mmap *map, uint64_t idx, uint64_t amt) {
	ASSERT (amt <= idx);
	do {
		map->r[idx] = map->r[idx + amt];
		idx++;
	} while (REGION_START (map->r[idx + amt - 1]));
}

// String representation for memory type (for debugging purposes)
const char* _regtype_str(uint32_t type) {
	switch (type) {
	case REGION_TYPE_NONE:
		return "None";
	case REGION_TYPE_AVAIL:
		return "Available";
	case REGION_TYPE_MULTIBOOT2:
		return "Multiboot2 Table";
	case REGION_TYPE_KERNEL:
		return "Kernel";
	case REGION_TYPE_ACPI_RECLAIM:
		return "ACPI Reclaimable";
	case REGION_TYPE_ACPI_NVS:
		return "ACPI Non Volatile";
	case REGION_TYPE_RSVD:
		return "Reserved";
	default:
		return "<Unknown>";
	}
}

// Insert a region into a memory map
// The memory map is stored in decreasing order of memory addresses
void mmap_insert_region(struct mmap *map, uint64_t start, uint64_t end, uint32_t type) {
	uint64_t endidx, startidx, regtyp;
	bool endaligned = false;
	if (start == end) {
		return;
	}
	ASSERT (map);
	ASSERT (!(start & ~PADDR_ALGN_MASK) && !(end & ~PADDR_ALGN_MASK));
	_find_idx (map, start, end, &endidx, &startidx);
	// As long as we're spread across many regions
	while (endidx < startidx) {
		if (REGION_TYPE (map->r[endidx]) >= type) {
			endidx++;
			endaligned = true;
			continue;
		}
		if (endaligned) {
			if (REGION_TYPE (map->r[endidx + 1]) == type) {
				_move_left (map, endidx + 1, 1);
				startidx--;
				continue;
			}
			REGION_SET_TYPE (map->r[endidx], type);
			endidx++;
			continue;
		}
		if (REGION_TYPE (map->r[endidx + 1]) != type) {
			_move_right (map, endidx, 1);
			startidx++;
			map->r[endidx + 1] = REGION_NEW (REGION_START (map->r[endidx]), type);
			REGION_SET_START (map->r[endidx], end);
			endaligned = true;
			endidx += 2;
		}
		REGION_SET_START (map->r[endidx], end);
		endaligned = true;
		endidx++;
	}
	// We're contained within one region
	if (REGION_TYPE (map->r[endidx]) >= type) {
		return;
	}
	if (endaligned) {
		if (REGION_START (map->r[startidx]) == start) {
			REGION_SET_TYPE (map->r[startidx], type);
			return;
		}
		_move_right (map, startidx, 1);
		map->r[startidx] = REGION_NEW (start, type);
		return;
	}
	if (REGION_START (map->r[startidx]) == start) {
		_move_right (map, startidx, 1);
		REGION_SET_START (map->r[startidx], end);
		map->r[startidx + 1] = REGION_NEW (start,  type);
		return;
	}
	_move_right (map, startidx, 2);
	map->r[startidx + 1] = REGION_NEW (start,  type);
	REGION_SET_START (map->r[startidx], end);
}

// Split a memory map at the given address. Splits any regions covering the address into two new
// regions.
void mmap_split_at(struct mmap *map, uint64_t addr) {
	uint64_t i;
	ASSERT (map);
	ASSERT (addr <= PADDR_ALGN_MASK);
	ASSERT (!(addr & ~PADDR_ALGN_MASK));
	for (i = 0; i < MMAP_MAX_NUM_ENTRIES; i++) {
		if (REGION_START (map->r[i]) == addr) {
			return;
		}
		if (REGION_START (map->r[i]) > addr) {
			continue;
		}
		_move_right (map, i, 1);
		REGION_SET_START (map->r[i], addr);
		return;
	}
	PANIC ("unreachable");
}

// Print the memory map
void mmap_print(const struct mmap *map) {
	uint32_t i;
	klog ("REGIONS:\n");
	for (i = 0; REGION_START (map->r[i]); i++) {
		klog ("  B: %#16llx | T: %s\n",
		      REGION_START (map->r[i]),
		      _regtype_str (REGION_TYPE (map->r[i])));
	}
	klog ("  B: %#16llx | T: %s\n",
	      REGION_START (map->r[i]),
	      _regtype_str (REGION_TYPE (map->r[i])));
}

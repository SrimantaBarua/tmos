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
	uint64_t len = 0, i;
	ASSERT (amt <= idx);
	for (i = idx; REGION_START (map->r[i]); i++);
	len = i + 1 - idx;
	for (i = idx + len; len; i--, len--) {
		map->r[i - 1] = map->r[i + amt - 1];
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
		map->r[startidx + 1] = map->r[startidx];
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
	map->r[startidx + 2] = map->r[startidx];
	map->r[startidx + 1] = REGION_NEW (start,  type);
	REGION_SET_START (map->r[startidx], end);
}

// Split a memory map at the given address. Splits any regions covering the address into two new
// regions.
void mmap_split_at(struct mmap *map, uint64_t addr);

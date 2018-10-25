// (C) 2018 Srimanta Barua
// Bitmap-based physical memory managers

#include <shuos/klog.h>
#include <shuos/memory.h>
#include <string.h>
#include <shuos/ds/bitmap.h>

// Bitmap-based physical memory manager
struct bm_pmmgr {
	paddr_t base, mem_sz, tot_blk, used_blk, fast_start, fast_end;
	struct bitmap bm0, bm1;
};

// TODO: Locking
static struct bm_pmmgr _mgr = { 0 };

// Set a bit in memory manager's bitmap, and also upper level bitmap if required
static void _set(paddr_t bit) {
	BM_SET(_mgr.bm0, bit);
	bit >>= WORD_SIZE_SHIFT;
	if (_mgr.bm0.map[bit] == WORD_MAX) {
		BM_SET(_mgr.bm1, bit);
	}
}

// Unet a bit in memory manager's bitmap, and also upper level bitmap if required
static void _unset(paddr_t bit) {
	BM_UNSET(_mgr.bm0, bit);
	bit >>= WORD_SIZE_SHIFT;
	if (_mgr.bm0.map[bit] == 0) {
		BM_UNSET(_mgr.bm1, bit);
	}
}

// Mark memory as used
static void _mark_used(paddr_t start, paddr_t end) {
	ASSERT(start >= _mgr.base);
	start = (start - _mgr.base) >> PAGE_SIZE_SHIFT;
	end = (end - _mgr.base) >> PAGE_SIZE_SHIFT;
	ASSERT(end <= _mgr.tot_blk);
	while (start < end) {
		if (!BM_TEST(_mgr.bm0, start)) {
			_set(start);
			_mgr.used_blk++;
		}
		start++;
	}
}

// Mark memory as free
static void _mark_free(paddr_t start, paddr_t end) {
	ASSERT(start >= _mgr.base);
	start = (start - _mgr.base) >> PAGE_SIZE_SHIFT;
	end = (end - _mgr.base) >> PAGE_SIZE_SHIFT;
	ASSERT(end <= _mgr.tot_blk);
	while (start < end) {
		if (BM_TEST(_mgr.bm0, start)) {
			_unset(start);
			_mgr.used_blk--;
		}
		start++;
	}
}

// Initialize a bitmap based memory manager
static void _init(region_t *regions, uint32_t num_regions, paddr_t fast_start, paddr_t fast_end) {
	paddr_t bm0_nbi, bm0_nby, bm1_nbi, bm1_nby, bm_sz;
	uint32_t i, bmreg = 0;
	ASSERT(num_regions > 1);
	// Get memory size
	_mgr.base = REGION_START(regions[num_regions - 1]);
	_mgr.mem_sz = REGION_START(regions[0]) - _mgr.base;
	_mgr.tot_blk = _mgr.used_blk = _mgr.mem_sz >> PAGE_SIZE_SHIFT;
	_mgr.fast_start = fast_start;
	_mgr.fast_end = fast_end;
	// Get size of bitmaps required
	bm0_nbi = _mgr.tot_blk;
	bm0_nby = ROUND_UP(bm0_nbi, WORD_SIZE) >> 3;
	bm1_nbi = bm0_nby >> (WORD_SIZE_SHIFT - 3);
	bm1_nby = ROUND_UP(bm1_nbi, WORD_SIZE) >> 3;
	// Get size we need to reserve within our regions
	bm_sz = PAGE_ALGN_UP(bm0_nby + bm1_nby);
	// Find a region big enough to accomodate the bitmaps. Also mark the regions as managed
	for (i = num_regions - 1; i > 0; i--) {
		REGION_SET_MANAGED(regions[i]);
		if (REGION_START(regions[i - 1]) - REGION_START(regions[i]) < bm_sz) {
			continue;
		}
		bmreg = i;
		break;
	}
	if (bmreg == 0) {
		PANIC("No space for bitmap in provided regions");
	}
	// Found
#if defined(__CFG_ARCH_x86_64__) || defined(__CFG_ARCH_x86__)
	BM_INIT(_mgr.bm0, REGION_START(regions[bmreg]) + KRNL_VBASE, bm0_nbi);
	BM_INIT(_mgr.bm1, REGION_START(regions[bmreg]) + KRNL_VBASE + BM_SZ(_mgr.bm0), bm1_nbi);
#else
	PANIC("Architecture not handled yet");
#endif
	memset((void*) REGION_START(regions[bmreg]) + KRNL_VBASE, 0xff, bm_sz);
	// Go over regions and mark map regions accordingly
	for (i = 1; i < num_regions; i++) {
		if (REGION_TYPE(regions[i]) == REGION_TYPE_AVAIL) {
			_mark_free(REGION_START(regions[i]), REGION_START(regions[i - 1]));
		}
	}
	// Mark bitmaps as used
	_mark_used(REGION_START(regions[bmreg]), REGION_START(regions[bmreg]) + bm_sz);
}

// Allocate one frame (for fast allocator)
static paddr_t _alloc() {
	paddr_t i, j, bm1_nw, bm0_nw, bm0_startw, bm0_endw, bm1_startw, bm1_endw;
	if (_mgr.used_blk == _mgr.tot_blk) {
		return PADDR_INVALID;
	}
	bm0_nw = ROUND_UP(_mgr.tot_blk, WORD_SIZE) >> WORD_SIZE_SHIFT;
	bm1_nw = ROUND_UP(bm0_nw, WORD_SIZE) >> WORD_SIZE_SHIFT;

	// We should know our limits
	bm0_startw = ROUND_UP(_mgr.fast_start >> PAGE_SIZE_SHIFT, WORD_SIZE) >> WORD_SIZE_SHIFT;
	bm0_endw = ROUND_DOWN(_mgr.fast_end >> PAGE_SIZE_SHIFT, WORD_SIZE) >> WORD_SIZE_SHIFT;
	bm1_startw = ROUND_UP(bm0_startw, WORD_SIZE) >> WORD_SIZE_SHIFT;
	bm1_endw = ROUND_DOWN(bm0_endw, WORD_SIZE) >> WORD_SIZE_SHIFT;

	// Find bm1 word which is not full
	for (i = bm1_startw; i < bm1_nw && i < bm1_endw; i++) {
		if (_mgr.bm1.map[i] == WORD_MAX) {
			continue;
		}
		// Found bm1 word. Found bit which is clear
		for (j = 0; j < WORD_SIZE; j++) {
			if ((_mgr.bm1.map[i] & (1 << j)) == 0) {
				break;
			}
		}
		// We know j < WORD_SIZE. Found bm0 word. Find bit in that word
		i = (i << WORD_SIZE_SHIFT) + j;
		for (j = 0; j < WORD_SIZE; j++) {
			if ((_mgr.bm0.map[i] & (1 << j)) == 0) {
				// Found frame
				i = (i << WORD_SIZE_SHIFT) + j;
				_set(i);
				return (i << PAGE_SIZE_SHIFT) + _mgr.base;
			}
		}
		PANIC("unreachable");
	}
	return PADDR_INVALID;
}

// Free one frame (for fast allocator)
static void _free(paddr_t addr) {
	ASSERT((addr & ~PADDR_ALGN_MASK) == 0);
	ASSERT(addr >= _mgr.fast_start);
	ASSERT(addr < _mgr.fast_end);
	addr >>= PAGE_SIZE_SHIFT;
	ASSERT(BM_TEST(_mgr.bm0, addr));
	_unset(addr);
}

// Remap the space taken by the bitmap
#if defined(__CFG_ARCH_x86_64__)
#include <shuos/arch/x86_64/memory.h>

static void _remap_cb() {
	uint64_t bm0sz, bm1sz, num;
	bm0sz = ROUND_UP(_mgr.tot_blk, WORD_SIZE) >> 3;
	bm1sz = ROUND_UP(bm0sz >> 3, WORD_SIZE) >> 3;
	num = PAGE_ALGN_UP(bm0sz +  bm1sz) >> PAGE_SIZE_SHIFT;
	vmm_map_to((vaddr_t) _mgr.bm0.map, (paddr_t) _mgr.bm0.map - KRNL_VBASE, num,
		    PTE_FLG_PRESENT | PTE_FLG_WRITABLE);
}

#endif

// The memory manager
struct pmmgr BM_PMMGR = {
	.init = _init,
	.alloc = _alloc,
	.free = _free,
#if defined(__CFG_ARCH_x86_64__)
	.remap_cb = _remap_cb,
#endif
};

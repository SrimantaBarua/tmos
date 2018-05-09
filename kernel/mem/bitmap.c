// (C) 2018 Srimanta Barua
// Bitmap-based physical memory managers

#include <klog.h>
#include <memory.h>
#include <string.h>
#include <ds/bitmap.h>

// Bitmap-based physical memory manager
struct bm_pmmgr {
	paddr_t base, mem_sz, tot_blk, used_blk;
	struct bitmap bm0, bm1;
};

static struct bm_pmmgr _fast_mgr = { 0 }, _spl_mgr = { 0 };

// Set a bit in memory manager's bitmap, and also upper level bitmap if required
static void _set(struct bm_pmmgr *mgr, paddr_t bit) {
	BM_SET (mgr->bm0, bit);
	bit >>= WORD_SIZE_SHIFT;
	if (mgr->bm0.map[bit] == WORD_MAX) {
		BM_SET (mgr->bm1, bit);
	}
}

// Unet a bit in memory manager's bitmap, and also upper level bitmap if required
static void _unset(struct bm_pmmgr *mgr, paddr_t bit) {
	BM_UNSET (mgr->bm0, bit);
	bit >>= WORD_SIZE_SHIFT;
	if (mgr->bm0.map[bit] == 0) {
		BM_UNSET (mgr->bm1, bit);
	}
}

// Mark memory as used
static void _mark_used(struct bm_pmmgr *mgr, paddr_t start, paddr_t end) {
	start = (start - mgr->base) >> PAGE_SIZE_SHIFT;
	end = (end - mgr->base) >> PAGE_SIZE_SHIFT;
	while (start < end) {
		if (!BM_TEST (mgr->bm0, start)) {
			_set (mgr, start);
			mgr->used_blk++;
		}
		start++;
	}
}

// Mark memory as free
static void _mark_free(struct bm_pmmgr *mgr, paddr_t start, paddr_t end) {
	start = (start - mgr->base) >> PAGE_SIZE_SHIFT;
	end = (end - mgr->base) >> PAGE_SIZE_SHIFT;
	while (start < end) {
		if (BM_TEST (mgr->bm0, start)) {
			_unset (mgr, start);
			mgr->used_blk--;
		}
		start++;
	}
}

// Initialize a bitmap based memory manager
static void _init(struct bm_pmmgr *mgr, region_t *regions, uint32_t num_regions) {
	paddr_t bm0_nbi, bm0_nby, bm1_nbi, bm1_nby, bm_sz;
	uint32_t i, bmreg = 0;
	ASSERT (num_regions > 1);
	// Get memory size
	mgr->base = REGION_START (regions[num_regions - 1]);
	mgr->mem_sz = REGION_START (regions[0]) - mgr->base;
	mgr->tot_blk = mgr->used_blk = mgr->mem_sz >> PAGE_SIZE_SHIFT;
	// Get size of bitmaps required
	bm0_nbi = mgr->mem_sz >> PAGE_SIZE_SHIFT;
	bm0_nby = ROUND_UP (bm0_nbi, WORD_SIZE) >> 3;
	bm1_nbi = bm0_nby >> (WORD_SIZE_SHIFT - 3);
	bm1_nby = ROUND_UP (bm1_nbi, WORD_SIZE) >> 3;
	// Get size we need to reserve within our regions
	bm_sz = PAGE_ALGN_UP (bm0_nby + bm1_nby);
	// Find a region big enough to accomodate the bitmaps
	for (i = num_regions - 1; i > 0; i--) {
		if (REGION_START (regions[i - 1]) - REGION_START (regions[i]) < bm_sz) {
			continue;
		}
		bmreg = i;
		break;
	}
	if (bmreg == 0) {
		PANIC ("No space for bitmap in provided regions");
	}
	// Found
#if defined(__ARCH_x86_64__) || defined(__ARCH_x86__)
	BM_INIT (mgr->bm0, REGION_START (regions[bmreg]) + KRNL_VBASE, bm0_nbi);
	BM_INIT (mgr->bm1, REGION_START (regions[bmreg]) + KRNL_VBASE + BM_SZ (mgr->bm0), bm1_nbi);
#else
	PANIC ("Architecture not handled yet");
#endif
	memset ((void*) REGION_START (regions[bmreg]) + KRNL_VBASE, 0xff, bm_sz);
	// Go over regions and mark map regions accordingly
	for (i = 1; i < num_regions; i++) {
		if (REGION_TYPE (regions[i]) == REGION_TYPE_AVAIL) {
			_mark_free (mgr, REGION_START (regions[i]), REGION_START (regions[i - 1]));
		}
	}
	// Mark bitmaps as used
	_mark_used (mgr, REGION_START (regions[bmreg]), REGION_START (regions[bmreg]) + bm_sz);
	klog ("mem_sz: %#llx | tot_blk: %#llx | used_blk: %#llx\n",
		mgr->mem_sz, mgr->tot_blk, mgr->used_blk);
}

// Initialize the fast bitmap based memory manager
static void _fast_init(region_t *regions, uint32_t num_regions) {
	_init (&_fast_mgr, regions, num_regions);
}

// Initialize the special bitmap based memory manager
static void _spl_init(region_t *regions, uint32_t num_regions) {
	_init (&_spl_mgr, regions, num_regions);
}

// The special memory manager
struct pmmgr BM_SPL_PMMGR = {
	.init = _spl_init,
};

// The fast memory manager
struct pmmgr BM_PMMGR = {
	.init = _fast_init,
};

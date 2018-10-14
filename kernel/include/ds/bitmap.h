// (C) 2018 Srimanta Barua
//
// A utility bitmap structure

#pragma once

#include <system.h>

// The bitmap structure
struct bitmap {
	word_t *map;
	word_t num_bits;
};

// Initialize a bitmap
#define BM_INIT(bm, a, nb) do { \
	(bm).map = (word_t*) (a);   \
	(bm).num_bits = (nb);    \
} while (0)

// Create a new bitmap
#define BM_NEW(addr, nb) \
	{ .map = (word_t*) (addr), .num_bits = (nb) }

// Set a bit in the bitmap(doesn't handle error)
#define BM_SET(bm, bit) do { \
	(bm).map[(bit) >> WORD_SIZE_SHIFT] |= (word_t) 1 << ((bit) & (WORD_SIZE - 1)); \
} while (0);

// Unset a bit in the bitmap(doesn't handle error)
#define BM_UNSET(bm, bit) do { \
	(bm).map[(bit) >> WORD_SIZE_SHIFT] &= ~((word_t) 1 << ((bit) & (WORD_SIZE - 1))); \
} while (0);

// Test a bit in the bitmap(doesn't handle error)
#define BM_TEST(bm, bit) \
	(((bm).map[(bit) >> WORD_SIZE_SHIFT] & ((word_t) 1 << ((bit) & (WORD_SIZE - 1)))) != 0)

// Get size of bitmap in bytes
#define BM_SZ(bm) \
	((ROUND_UP((bm).num_bits, WORD_SIZE)) >> 3)

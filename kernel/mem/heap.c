// (C) 2018 Srimanta Barua
//
// Linked-list based heap.
//
// We maintain lists of chunks of different sizes. For an allocation request, we check if a chunk
// just larger than the request is empty. If yes, we allocate. If not, we move to chunks of
// progressively bigger sizes till we find one that is empty. We then break the chunk, append the
// leftover to a list of the correct size, and return the allocated memory.
//
// On freeing, we check if adjacent chunks are free. If yes, we merge the chunks and move them to
// the appropriate list.

#include <shuos/memory.h>
#include <shuos/system.h>
#include <string.h>
#include <stdbool.h>
#include <shuos/spin.h>
#include <shuos/klog.h>
#include <shuos/ds/list.h>

// TODO: Per-CPU lockless memory pools

// Head of a chunk of memory. In case of a free chunk, it stores size, pointer to next free,
// pointer to previous free. In case of used chunk, it stores just the size. The LSB of the size
// field is used to denote if the chunk is used or not. The 2nd LSB is used to denote if the
// previous chunk is used or not
struct heap_chunk {
	word_t memsz;
	struct list list;
};

// Footer of a heap chunk. This is only present on unused chunks.
struct heap_footer {
	word_t memsz;
};

// Is a chunk used
static inline bool _chunk_is_used(const struct heap_chunk *chunk) {
	return (chunk->memsz & 1) != 0;
}

// Is a chunk's previous chunk (in terms of address) used?
static inline bool _chunk_is_prev_used(const struct heap_chunk *chunk) {
	return (chunk->memsz & 2) != 0;
}

// Denote a chunk as used
static inline void _chunk_set_used(struct heap_chunk *chunk) {
	chunk->memsz |= (word_t) 1;
}

// Denote a chunk as free
static inline void _chunk_set_free(struct heap_chunk *chunk) {
	chunk->memsz &= ~(word_t) 1;
}

// Denote a chunk's previous chunk as used
static inline void _chunk_set_prev_used(struct heap_chunk *chunk) {
	chunk->memsz |= (word_t) 2;
}

// Denote a chunk's previous chunk as free
static inline void _chunk_set_prev_free(struct heap_chunk *chunk) {
	chunk->memsz &= ~(word_t) 2;
}

// Get the memory size for the chunk
static inline word_t _chunk_memsz(const struct heap_chunk *chunk) {
	return chunk->memsz & ~7;
}

// Get the size of the whole chunk
static inline word_t _chunk_sz(const struct heap_chunk *chunk) {
	return _chunk_memsz(chunk) + (WORD_SIZE >> 3);
}

// Get a pointer to the footer of the given chunk. It is assumed it is free
static inline struct heap_footer* _chunk_footer(struct heap_chunk *chunk) {
	return (struct heap_footer*) ((void*) chunk + _chunk_memsz(chunk));
}

// Get a pointer to the header of the chunk given a pointer to its footer
static inline struct heap_chunk* _chunk_header_from_footer(struct heap_footer *footer) {
	return (struct heap_chunk*) ((void*) footer - footer->memsz);
}

// Get a pointer to previous chunk in list
static inline struct heap_chunk* _chunk_list_prev(struct heap_chunk *chunk) {
	return container_of(chunk->list.prev, struct heap_chunk, list);
}

// Get a pointer to next chunk in list
static inline struct heap_chunk* _chunk_list_next(struct heap_chunk *chunk) {
	return container_of(chunk->list.next, struct heap_chunk, list);
}

// Get memory size of previous chunk. Only works if previous chunk is free
static inline word_t _chunk_addr_prev_memsz(const struct heap_chunk *chunk) {
	return ((struct heap_footer*) ((void*) chunk - sizeof(struct heap_footer*)))->memsz;
}

// Get full size of previous chunk. Only works if previous chunk is free
static inline word_t _chunk_addr_prev_sz(const struct heap_chunk *chunk) {
	return _chunk_addr_prev_memsz(chunk) + (WORD_SIZE >> 3);
}

// Get a pointer to previous chunk in terms of memory address. Only words if previous chunk is free
static inline struct heap_chunk* _chunk_addr_prev(struct heap_chunk *chunk) {
	if (_chunk_is_prev_used(chunk)) {
		return (struct heap_chunk*) ((void*) chunk - _chunk_addr_prev_sz(chunk));
	}
	return NULL;
}

// Get a pointer to next chunk in  terms of memory address
static inline struct heap_chunk* _chunk_addr_next(struct heap_chunk *chunk) {
	return (struct heap_chunk*) ((void*) chunk + _chunk_sz(chunk));
}

// Split off from the beginning of a chunk. Assumes chunk is free. Pointer points to new beginning
// of right chunk. Returns pointer to left chunk. Assumes enough space
static struct heap_chunk* _chunk_split_front(struct heap_chunk **chunk, word_t left_memsz) {
	struct heap_chunk *left, *right;
	left = *chunk;
	right = (struct heap_chunk *) ((void*) left + left_memsz + (WORD_SIZE >> 3));
	right->memsz = _chunk_memsz(left) - left_memsz - (WORD_SIZE >> 3);
	left->memsz = left_memsz | (left->memsz & 7);
	_chunk_footer(left)->memsz = left_memsz;
	_chunk_footer(right)->memsz = _chunk_memsz(right);
	list_add_front(&left->list, &right->list);
	*chunk = right;
	return left;
}

// Split off from the end of a chunk. Assumes chunk is free. Pointer points to new beginning
// of left chunk. Returns pointer to right chunk. Assumes enough space
static struct heap_chunk* _chunk_split_rear(struct heap_chunk *chunk, word_t left_memsz) {
	struct heap_chunk *left, *right;
	left = chunk;
	right = (struct heap_chunk *) ((void*) left + left_memsz + (WORD_SIZE >> 3));
	right->memsz = _chunk_memsz(left) - left_memsz - (WORD_SIZE >> 3);
	left->memsz = left_memsz | (left->memsz & 7);
	_chunk_footer(left)->memsz = left_memsz;
	_chunk_footer(right)->memsz = _chunk_memsz(right);
	list_add_front(&left->list, &right->list);
	return right;
}

// Bins. List of memory chunks of a fixed size
struct heap_bin {
	word_t memsz;           // Size of memory in this bin
	struct heap_chunk head; // Head of list of chunks
};

// Number of bin sizes
#define HEAP_NUM_BINS 54

// Array of list of bins
static struct heap_bin _bins[HEAP_NUM_BINS] = {
	// Increment by 8
	{ 16, { 0, LIST_INIT(_bins[0].head.list) } },
	{ 24, { 0, LIST_INIT(_bins[1].head.list) } },
	{ 32, { 0, LIST_INIT(_bins[2].head.list) } },
	{ 40, { 0, LIST_INIT(_bins[3].head.list) } },
	{ 48, { 0, LIST_INIT(_bins[4].head.list) } },
	{ 56, { 0, LIST_INIT(_bins[5].head.list) } },
	{ 64, { 0, LIST_INIT(_bins[6].head.list) } },
	{ 72, { 0, LIST_INIT(_bins[7].head.list) } },
	{ 80, { 0, LIST_INIT(_bins[8].head.list) } },
	{ 88, { 0, LIST_INIT(_bins[9].head.list) } },
	{ 96, { 0, LIST_INIT(_bins[10].head.list) } },
	{ 104, { 0, LIST_INIT(_bins[11].head.list) } },
	{ 112, { 0, LIST_INIT(_bins[12].head.list) } },
	{ 120, { 0, LIST_INIT(_bins[13].head.list) } },
#define INC8_END 128
	// Increment by 16
	{ 128, { 0, LIST_INIT(_bins[14].head.list) } },
	{ 144, { 0, LIST_INIT(_bins[15].head.list) } },
	{ 160, { 0, LIST_INIT(_bins[16].head.list) } },
	{ 176, { 0, LIST_INIT(_bins[17].head.list) } },
	{ 192, { 0, LIST_INIT(_bins[18].head.list) } },
	{ 208, { 0, LIST_INIT(_bins[19].head.list) } },
	{ 224, { 0, LIST_INIT(_bins[20].head.list) } },
	{ 240, { 0, LIST_INIT(_bins[21].head.list) } },
#define INC16_END 256
	// Increment by 32
	{ 256, { 0, LIST_INIT(_bins[22].head.list) } },
	{ 288, { 0, LIST_INIT(_bins[23].head.list) } },
	{ 320, { 0, LIST_INIT(_bins[24].head.list) } },
	{ 352, { 0, LIST_INIT(_bins[25].head.list) } },
	{ 384, { 0, LIST_INIT(_bins[26].head.list) } },
	{ 416, { 0, LIST_INIT(_bins[27].head.list) } },
	{ 448, { 0, LIST_INIT(_bins[28].head.list) } },
	{ 480, { 0, LIST_INIT(_bins[29].head.list) } },
#define INC32_END 512
	// Increment by 64
	{ 512, { 0, LIST_INIT(_bins[30].head.list) } },
	{ 576, { 0, LIST_INIT(_bins[31].head.list) } },
	{ 640, { 0, LIST_INIT(_bins[32].head.list) } },
	{ 704, { 0, LIST_INIT(_bins[33].head.list) } },
	{ 768, { 0, LIST_INIT(_bins[34].head.list) } },
	{ 832, { 0, LIST_INIT(_bins[35].head.list) } },
	{ 896, { 0, LIST_INIT(_bins[36].head.list) } },
	{ 960, { 0, LIST_INIT(_bins[37].head.list) } },
#define INC64_END 1024
	// Increment by 128
	{ 1024, { 0, LIST_INIT(_bins[38].head.list) } },
	{ 1152, { 0, LIST_INIT(_bins[39].head.list) } },
	{ 1280, { 0, LIST_INIT(_bins[40].head.list) } },
	{ 1408, { 0, LIST_INIT(_bins[41].head.list) } },
	{ 1536, { 0, LIST_INIT(_bins[42].head.list) } },
	{ 1664, { 0, LIST_INIT(_bins[43].head.list) } },
	{ 1792, { 0, LIST_INIT(_bins[44].head.list) } },
	{ 1920, { 0, LIST_INIT(_bins[45].head.list) } },
#define INC128_END 2048
	// Increment by 256
	{ 2048, { 0, LIST_INIT(_bins[46].head.list) } },
	{ 2304, { 0, LIST_INIT(_bins[47].head.list) } },
	{ 2560, { 0, LIST_INIT(_bins[48].head.list) } },
	{ 2816, { 0, LIST_INIT(_bins[49].head.list) } },
	{ 3072, { 0, LIST_INIT(_bins[50].head.list) } },
	{ 3328, { 0, LIST_INIT(_bins[51].head.list) } },
	{ 3584, { 0, LIST_INIT(_bins[52].head.list) } },
	{ 3840, { 0, LIST_INIT(_bins[53].head.list) } },
#define INC256_END 3840
};

#if WORD_SIZE == 32
#define BIN_MIN_SIZE 16
#else
#define BIN_MIN_SIZE 24
#endif

// Get the mininmum bin size for a given request size
static word_t _get_size(size_t req) {
	if (req <= BIN_MIN_SIZE) {
		return BIN_MIN_SIZE;
	}
	// TODO: Handle requests larger than 3840 bytes
	ASSERT(req <= INC256_END);
	if (req <= INC8_END) {
		return ROUND_UP(req, 8);
	}
	if (req <= INC16_END) {
		return ROUND_UP(req, 16);
	}
	if (req <= INC32_END) {
		return ROUND_UP(req, 32);
	}
	if (req <= INC64_END) {
		return ROUND_UP(req, 64);
	}
	if (req <= INC128_END) {
		return ROUND_UP(req, 128);
	}
	if (req <= INC256_END) {
		return ROUND_UP(req, 256);
	}
	PANIC("unreachable");
}

// Get bin index for given request size
static size_t _get_bin_idx(size_t binsz) {
	size_t base = 0;
	if (binsz <= INC8_END) {
		return (binsz >> 3) - 1;
	}
	base = (INC8_END >> 3) - 1;
	if (binsz <= INC16_END) {
		return base + ((binsz - INC8_END) >> 4);
	}
	base += (INC16_END - INC8_END) >> 4;
	if (binsz <= INC32_END) {
		return base + ((binsz - INC16_END) >> 5);
	}
	base += (INC32_END - INC16_END) >> 5;
	if (binsz <= INC64_END) {
		return base + ((binsz - INC32_END) >> 6);
	}
	base += (INC64_END - INC32_END) >> 6;
	if (binsz <= INC128_END) {
		return base + ((binsz - INC64_END) >> 7);
	}
	base += (INC128_END - INC64_END) >> 7;
	if (binsz <= INC256_END) {
		return base + ((binsz - INC128_END) >> 8);
	}
	// TODO: Handle requests larger than 3840 bytes
	PANIC("unreachable");
}

// Store global heap state
static struct {
	struct heap_chunk *last;
} _heap;

// Get current end of heap
static void* _heap_cur_end() {
	return (void*) _heap.last + (WORD_SIZE >> 3) + _heap.last->memsz;
}

// Initialize the kernel heap
void heap_init() {
	// We know brk is initially at the beginning of the heap. So allocate one page
	_heap.last = (struct heap_chunk*) ksbrk(PAGE_SIZE << 1);
	_heap.last->memsz = (PAGE_SIZE << 1) - (WORD_SIZE >> 3);
	_chunk_set_free(_heap.last);
	_chunk_set_prev_used(_heap.last);
	list_init(&_heap.last->list);
}

// Allocate a block of memory of the given size
void* kmalloc(size_t size) {
	size_t bindx;
	struct heap_chunk *chunk;
	// 0-size malloc
	if (!size) {
		return NULL;
	}
	// Get bin index for size. TODO: Handle larger requests
	bindx = _get_bin_idx(_get_size(size));
	// Check if bin has free nodes. If yes, allocate
	if (!list_is_empty(&_bins[bindx].head.list)) {
		// First chunk in list
		chunk = _chunk_list_next(&_bins[bindx].head);
		_chunk_set_used(chunk);
		if (chunk->list.next != &_bins[bindx].head.list) {
			_chunk_set_prev_used(_chunk_addr_next(chunk));
		}
		list_del(&chunk->list);
		return (void*) chunk + (WORD_SIZE >> 3);
	}
	// No free nodes. Break off from last chunk. Check if it is big enough
	if (_chunk_memsz(_heap.last) - _bins[bindx].memsz < PAGE_SIZE) {
		// Need to push chunk back. TODO: Handle larger requests
		ksbrk(PAGE_SIZE);
		_heap.last->memsz += PAGE_SIZE;
	}
	// Enough space in last chunk. Break off
	chunk = _chunk_split_front(&_heap.last, _bins[bindx].memsz);
	_chunk_set_used(chunk);
	return (void*) chunk + (WORD_SIZE >> 3);
}

// Allocate a block of memory of the given size, and zero it out
void* kcalloc(size_t nmemb, size_t size) {
	void *ptr;
	size_t binsz;
	if (!nmemb || !size) {
		return NULL;
	}
	binsz = _get_size(nmemb * size);
	ptr = kmalloc(binsz);
	memset(ptr, 0, binsz);
	return ptr;
}

// Reallocate a block of memory to be of the given size, and copy the contents
void* krealloc(void *ptr, size_t size);

// Free allocated memory
void kfree(void *ptr) {
	struct heap_chunk *chunk;
	struct heap_footer *footer;
	size_t bindx;
	ASSERT((uintptr_t) ptr > KRNL_HEAP_START && ptr < _heap_cur_end());
	chunk = (struct heap_chunk*) (ptr - (WORD_SIZE >> 3));
	if (!_chunk_is_used(chunk)) {
		PANIC("Attempt to free unallocated memory\n");
	}
	bindx = _get_bin_idx(_chunk_memsz(chunk));
	// Free the chunk and add to beginning of bin list
	_chunk_set_free(chunk);
	footer = _chunk_footer(chunk);
	footer->memsz = _chunk_memsz(chunk);
	list_add_front(&_bins[bindx].head.list, &chunk->list);
}

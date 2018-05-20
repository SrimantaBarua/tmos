// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <static_assert.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERISON_PATCH 0

// Compiler options
#define FORCEINLINE inline __attribute__ ((always_inline))
#define NOINLINE    __attribute__ ((noinline))
#define PACKED      __attribute__ ((packed))
#define NORETURN    __attribute__ ((noreturn))

#if defined(__CFG_ARCH_x86_64__)
#include <arch/x86_64/system.h>
#endif

// Utility math
#define ROUND_UP(x, y) (((x) + ((y) - 1)) & ~((y) - 1))
#define ROUND_DOWN(x, y) ((x) & ~((y) - 1))
#define IS_ALIGNED(x, y) (((x) & ~((y) - 1)) == 0)

// Page-alignment macros
#define PAGE_ALGN_UP(x) ROUND_UP (x, PAGE_SIZE)
#define PAGE_ALGN_DOWN(x) ROUND_DOWN (x, PAGE_SIZE)

// Get if two entities are of the same type
#define is_same_type(x, y) (typeof (x) == typeof (y))

// Get offset of struct member
#define offset_of(type, member) ((size_t) (&((type *) 0)->member))

// Get pointer to struct containing the given member, given type of struct
#define container_of(ptr, type, member) ({ \
	void *__mptr = (void*) (ptr);      \
	STATIC_ASSERT (is_same_type (*(ptr), ((type *) 0)->member)); \
	((type *) (_mptr - offset_of (type, member))); })

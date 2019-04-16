// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <tmos/static_assert.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERISON_PATCH 0

#include <tmos/arch/system.h>

// Utility math
#define ROUND_UP(x, y) (((x) + ((y) - 1)) & ~((y) - 1))
#define ROUND_DOWN(x, y) ((x) & ~((y) - 1))
#define IS_ALIGNED(x, y) (((x) & ((y) - 1)) == 0)

// Page-alignment macros
#define PAGE_ALGN_UP(x) ROUND_UP(x, PAGE_SIZE)
#define PAGE_ALGN_DOWN(x) ROUND_DOWN(x, PAGE_SIZE)

// Check at compile time if something is of a given type. Always evaluates to 1
#define typecheck(type, x)                       \
	({                                       \
		type __dummy1;                   \
		typeof(x) __dummy2;             \
		(void) (&__dummy1 == &__dummy2); \
		1;                               \
	 })

// Get if two entities are of the same type
#define is_same_type(x, y) typecheck(typeof(x), y)

// Get offset of struct member
#define offset_of(type, member) ((size_t) (&((type *) 0)->member))

// Get pointer to struct containing the given member, given type of struct
#define container_of(ptr, type, member) ({ \
	void *__mptr = (void*) (ptr);      \
	is_same_type(*(ptr), ((type *) 0)->member); \
	((type *) (__mptr - offset_of(type, member))); })

// Stop forever
void __attribute__((noreturn)) crash_and_burn();

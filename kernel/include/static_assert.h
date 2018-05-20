// (C) 2018 Srimanta Barua
//
// Compile-time checks

#pragma once

#include <stdbool.h>

// Compile-time error with given message
#define __compiletime_error(msg) __attribute__((error(msg)))

// Compile-time warning with given message
#define __compiletime_warning(msg) __attribute__((warning(msg)))

// If the given compile-time assertion is false, generate given error message
#define STATIC_ASSERT_MSG(cond, msg)                                           \
	do {                                                                   \
		bool __cond = !(cond);                                         \
		extern void compiletime_error(void) __compiletime_error (msg); \
		if (__cond) {                                                  \
			compiletime_error();                                   \
		}                                                              \
	} while (0)


// Check condition at compile-time
#define STATIC_ASSERT(cond) STATIC_ASSERT_MSG (cond, "Assertion failed: " #cond)

// (C) 2018 Srimanta Barua
//
// Interface for logging data to a log buffer, console, or other output

#pragma once

// Write a log message
void klog(const char *fmt, ...);

// Kernel panic
void __panic(const char *file, unsigned line, const char *fn, const char *fmt, ...);

#define PANIC(fmt, ...) __panic(__FILE__, __LINE__, __func__, fmt, ## __VA_ARGS__)

#define ASSERT(x) { \
	if (!(x)) { \
		PANIC("Assertion failed: %s\n", #x); \
	} \
}


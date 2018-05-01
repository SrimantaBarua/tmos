// (C) 2018 Srimanta Barua
//
// Interface for logging data to a log buffer, console, or other output

#pragma once

// Kernel log type. This is a bit like linux 1.0's system
#define KLOG_CRIT  "<1>"  // Critical condition
#define KLOG_ERR   "<2>"  // Error
#define KLOG_WARN  "<3>"  // Warning
#define KLOG_INFO  "<4>"  // Information
#define KLOG_DEBUG "<5>"  // Debug messages

// Write a log message
void klog(const char *fmt, ...);

// Kernel panic
void __panic(const char *file, unsigned line, const char *fn, const char *fmt, ...);

#define PANIC(fmt, ...) __panic (__FILE__, __LINE__, __func__, fmt, __VA_ARGS__)

#define ASSERT(x) { \
	if (!(x)) { \
		PANIC("Assertion failed: %s", #x); \
	} \
}


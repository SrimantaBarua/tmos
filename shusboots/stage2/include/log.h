// (C) 2018 Srimanta Barua
//
// Interface for logging data to a log buffer, console, or other output

#pragma once

// Type of log message
enum log_type {
	LOG_INFO = 0,
	LOG_WARN = 1,
	LOG_ERR = 2,
	LOG_SUCCESS = 3,
};

// Write a log message
void log(enum log_type type, const char *fmt, ...);

// Log without log type (Useful for panics)
void __log_without_typ(const char *fmt, ...);

// Macros for asserts, panics etc
#define PANIC(fmt, ...) { \
	__log_without_typ("[PANIC]: %s:%d %s(): ", __FILE__, __LINE__, __func__); \
	__log_without_typ(fmt, __VA_ARGS__); \
	__asm__ __volatile__ ("cli; hlt; jmp $":::"memory"); \
}

#define ASSERT(x) { \
	if (!(x)) { \
		PANIC("Assertion failed: " #x); \
	} \
}

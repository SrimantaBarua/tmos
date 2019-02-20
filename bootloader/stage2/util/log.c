// (C) 2018 Srimanta Barua
//
// Functionality for logging

#include <log.h>
#include <serial.h>
#include <term.h>
#include <fmt.h>

// Get string for given log type
static const char* _log_type_str(enum log_type type) {
	switch (type) {
	case LOG_INFO:
		return "[INFO]: ";
	case LOG_WARN:
		return "[WARN]: ";
	case LOG_ERR:
		return "[ERR]: ";
	case LOG_SUCCESS:
		return "[SUCCESS]: ";
	}
	// Unreachable
	return "";
}

// Write a log message
void log(enum log_type type, const char *fmt, ...) {
	va_list ap;
	serial_write_str(_log_type_str(type));
	va_start(ap, fmt);
	fmt_write(serial_write_str, fmt, ap);
	va_end(ap);
}

// Log message without type
void __log_without_typ(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fmt_write(serial_write_str, fmt, ap);
	va_end(ap);
}

// Log a formatted string to screen
void vlog(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	fmt_write(term_write_str, fmt, ap);
	va_end(ap);
	display_flush();
}

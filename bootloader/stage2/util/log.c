// (C) 2018 Srimanta Barua
//
// Functionality for logging

#include <log.h>
#include <serial.h>
#include <term.h>
#include <vsprintf.h>


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


static char _log_buf[4096];


// Write a log message
void log(enum log_type type, const char *fmt, ...) {
	va_list ap;
	serial_write_str(_log_type_str(type));
	va_start(ap, fmt);
	vsnprintf(_log_buf, sizeof(_log_buf), fmt, ap);
	va_end(ap);
	serial_write_str(_log_buf);
}


// Log message without type
void __log_without_typ(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(_log_buf, sizeof(_log_buf), fmt, ap);
	va_end(ap);
	serial_write_str(_log_buf);
}


static char _vlog_buf[4096];


// Log a formatted string to screen
void vlog(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(_vlog_buf, sizeof(_vlog_buf), fmt, ap);
	va_end(ap);
	term_write_str(_vlog_buf);
	display_flush();
}

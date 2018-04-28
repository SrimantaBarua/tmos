// Functionality for logging

#include <log.h>
#include <serial.h>

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
void log (enum log_type type, const char *fmt, ...) {
	serial_write_str (COM1, _log_type_str (type));
	// TODO: Proper formatting
	serial_write_str (COM1, fmt);
}

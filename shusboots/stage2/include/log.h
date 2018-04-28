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

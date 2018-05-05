// (C) 2018 Srimanta Barua
//
// Functionality for logging

#include <klog.h>
#include <serial.h>
#include <vsprintf.h>
#include <stdbool.h>
#include <system.h>

#ifndef KLOG_BUF_SZ
#define KLOG_BUF_SZ 4096
#endif

// TODO: This should probably be locked
static char _log_buf[KLOG_BUF_SZ];

// Write a log message
void klog (const char *fmt, ...) {
	int vslen;
	va_list ap;

	bool int_was_enabled = sys_int_enabled ();
	sys_disable_int ();

	va_start (ap, fmt);
	vslen = vsnprintf (_log_buf, sizeof (_log_buf), fmt, ap);
	va_end (ap);
	if (vslen >= sizeof (_log_buf)) {
		vslen = sizeof (_log_buf) - 1;
	}
	serial_write_str (COM1, _log_buf);

	if (int_was_enabled) {
		sys_enable_int ();
	}
#if 0
	if (_log_buf[vslen - 1] != '\n') {
		serial_write_str (COM1, "\n");
	}
#endif
}

// Kernel panic
void __panic(const char *file, unsigned line, const char *fn, const char *fmt, ...) {
	int vslen;
	va_list ap;

	bool int_was_enabled = sys_int_enabled ();
	sys_disable_int ();

	klog ("[KERNEL PANIC]: %s:%u %s\n", file, line, fn);

	va_start (ap, fmt);
	vslen = vsnprintf (_log_buf, sizeof (_log_buf), fmt, ap);
	va_end (ap);
	if (vslen >= sizeof (_log_buf)) {
		vslen = sizeof (_log_buf) - 1;
	}
	serial_write_str (COM1, _log_buf);

	if (int_was_enabled) {
		sys_enable_int ();
	}

	crash_and_burn ();
}

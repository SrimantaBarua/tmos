// (C) 2018 Srimanta Barua
//
// Functionality for logging

#include <tmos/klog.h>
#include <tmos/serial.h>
#include <tmos/vsprintf.h>
#include <stdbool.h>
#include <tmos/spin.h>

#ifndef KLOG_BUF_SZ
#define KLOG_BUF_SZ 4096
#endif

// TODO: This should probably be locked
static char _log_buf[KLOG_BUF_SZ];
static spin_t _lock = SPIN_UNLOCKED;

// Write a log message
// TODO: Revisit when enabling SMP. Needs better locking, and we should not write to serial
// straightaway. Instead, we should have a local buffer, which we append to the global log buffer,
// which is then flushed to the debug console when required.
void klog(const char *fmt, ...) {
	int vslen;
	va_list ap;

	spin_lock_intsafe(&_lock);

	va_start(ap, fmt);
	vslen = vsnprintf(_log_buf, sizeof(_log_buf), fmt, ap);
	va_end(ap);
	if (vslen >= sizeof(_log_buf)) {
		vslen = sizeof(_log_buf) - 1;
	}
	serial_write_str(COM1, _log_buf);

	spin_unlock(&_lock);
#if 0
	if (_log_buf[vslen - 1] != '\n') {
		serial_write_str(COM1, "\n");
	}
#endif
}

// Kernel panic
// TODO: Again, revisit for SMP. See comment before klog()
void __panic(const char *file, unsigned line, const char *fn, const char *fmt, ...) {
	int vslen;
	va_list ap;

	spin_lock_intsafe(&_lock);

	klog("[KERNEL PANIC]: %s:%u %s\n", file, line, fn);

	va_start(ap, fmt);
	vslen = vsnprintf(_log_buf, sizeof(_log_buf), fmt, ap);
	va_end(ap);
	if (vslen >= sizeof(_log_buf)) {
		vslen = sizeof(_log_buf) - 1;
	}
	serial_write_str(COM1, _log_buf);

	spin_unlock(&_lock);

	crash_and_burn();
}

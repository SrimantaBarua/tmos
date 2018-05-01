// (C) 2018 Srimanta Barua
//
// Functionality for logging

#include <klog.h>
#include <serial.h>
#include <vsprintf.h>

#ifndef KLOG_BUF_SZ
#define KLOG_BUF_SZ 4096
#endif

// Write a log message
void klog (const char *fmt, ...) {
	static char log_buf[KLOG_BUF_SZ];
	int vslen;
	va_list ap;
	va_start (ap, fmt);
	vslen = vsnprintf (log_buf, sizeof (log_buf), fmt, ap);
	va_end (ap);
	if (vslen >= sizeof (log_buf)) {
		vslen = sizeof (log_buf) - 1;
	}
	serial_write_str (COM1, log_buf);
#if 0
	if (log_buf[vslen - 1] != '\n') {
		serial_write_str (COM1, "\n");
	}
#endif
}

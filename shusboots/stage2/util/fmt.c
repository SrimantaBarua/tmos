// (C) 2018 Srimanta Barua
//
// Text formatting utility

#include <fmt.h>
#include <stdint.h>
#include <fmtutil.h>

// To avoid redundant code
static void _flush_buf(void (*cb) (const char *str), char *buf, int *idx) {
	buf[*idx] = '\0';
	cb (buf);
	*idx = 0;
}

// Write formatted text using the callback
void fmt_write(void (*cb) (const char *str), const char *fmt, va_list ap) {
	char buf[32];
	const char *fmt_begun_at;
	int intlevel = 0, buf_idx = 0;
	while (*fmt) {
		if (fmt[0] != '%' || fmt[1] == '%') {
			if  (fmt[0] == '%') {
				fmt++;
			}
			for ( ; buf_idx < sizeof (buf) - 1; buf_idx++) {
				if (!fmt[buf_idx] || fmt[buf_idx] == '%') {
					break;
				}
				buf[buf_idx] = fmt[buf_idx];
			}
			fmt += buf_idx;
			_flush_buf (cb, buf, &buf_idx);
			continue;
		}
		fmt_begun_at = fmt;
		fmt++;
		switch (*fmt) {
		case 'c':
			fmt++;
			if (buf_idx == sizeof (buf) - 1) {
				_flush_buf (cb, buf, &buf_idx);
			}
			buf[buf_idx] = ((char) va_arg (ap, uint32_t));
			buf_idx++;
			continue;
		case 's':
			fmt++;
			if (buf_idx > 0) {
				_flush_buf (cb, buf, &buf_idx);
			}
			cb (va_arg (ap, const char *));
			continue;
		case 'l':
			fmt++;
			intlevel = 1;
			if (*fmt == 'l') {
				fmt++;
				intlevel = 2;
			}
			switch (*fmt) {
			case 'd':
			case 'u':
			case 'o':
			case 'x':
				break;
			default:
				if (buf_idx > 0) {
					_flush_buf (cb, buf, &buf_idx);
				}
				cb (fmt_begun_at);
				return;
			}
		case 'd':
		case 'u':
		case 'o':
		case 'x':
			if (buf_idx > 0) {
				_flush_buf (cb, buf, &buf_idx);
			}
			switch (*fmt) {
			case 'd':
				intlevel == 2
					? lltoa (va_arg (ap, int64_t), buf, 10)
					: itoa (va_arg (ap, int), buf, 10);
				break;
			case 'u':
				intlevel == 2
					? ulltoa (va_arg (ap, uint64_t), buf,  10)
					: utoa (va_arg (ap, unsigned), buf, 10);
				break;
			case 'o':
				intlevel == 2
					? ulltoa (va_arg (ap, int64_t), buf,  8)
					: utoa (va_arg (ap, unsigned), buf, 8);
				break;
			case 'x':
				intlevel == 2
					? ulltoa (va_arg (ap, int64_t), buf,  16)
					: utoa (va_arg (ap, unsigned), buf, 16);
				break;
			}
			cb (buf);
			fmt++;
			intlevel = 0;
			break;
		default:
			if (buf_idx > 0) {
				_flush_buf (cb, buf, &buf_idx);
			}
			cb (fmt_begun_at);
			return;
		}
	}
	if (buf_idx > 0) {
		_flush_buf (cb, buf, &buf_idx);
	}
}

// (C) 2018 Srimanta Barua
//
// Text formatting utility

#include <fmt.h>
#include <stdint.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <intutil.h>

#define ISDIGIT(x) ((x) >= '0' && (x) <= '9')

// To avoid redundant code
static void _flush_buf(void (*cb) (const char *str), char *buf, unsigned *idx) {
	buf[*idx] = '\0';
	cb(buf);
	*idx = 0;
}

// Write formatted text using the callback
void fmt_write(void (*cb) (const char *str), const char *fmt, va_list ap) {
	char buf[64], width_buf[32];
	const char *fmt_begun_at;
	unsigned intlevel = 0, buf_idx = 0, width = 0, i;
	bool size_override = false, zero_pad = false;
	while (*fmt) {
		if (fmt[0] != '%' || fmt[1] == '%') {
			if  (fmt[0] == '%') {
				fmt++;
			}
			for ( ; buf_idx < sizeof(buf) - 1; buf_idx++) {
				if (!*fmt || *fmt == '%') {
					break;
				}
				buf[buf_idx] = *fmt;
				fmt++;
			}
			_flush_buf(cb, buf, &buf_idx);
			continue;
		}
		fmt_begun_at = fmt;
		fmt++;
		if (ISDIGIT(*fmt)) {
			// Handle width specification, like %08llx
			if (*fmt == '0') {
				zero_pad = true;
				fmt++;
			} else {
				zero_pad = false;
			}
			width = 0;
			i = 0;
			size_override = true;
			while (ISDIGIT(*fmt) && i < sizeof(width_buf) - 1) {
				width_buf[i] = *fmt;
				fmt++;
				i++;
			}
			width_buf[i] = '\0';
			width = atoi(width_buf);
		} else {
			size_override = false;
		}
		switch (*fmt) {
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
					_flush_buf(cb, buf, &buf_idx);
				}
				cb(fmt_begun_at);
				return;
			}
		case 'd':
		case 'u':
		case 'o':
		case 'x':
			if (buf_idx > 0) {
				_flush_buf(cb, buf, &buf_idx);
			}
			switch (*fmt) {
			case 'd':
				intlevel == 2
					? lltoa(va_arg(ap, int64_t), buf, 10)
					: itoa(va_arg(ap, int), buf, 10);
				break;
			case 'u':
				intlevel == 2
					? ulltoa(va_arg(ap, uint64_t), buf, 10)
					: utoa(va_arg(ap, uint32_t), buf, 10);
				break;
			case 'o':
				intlevel == 2
					? ulltoa(va_arg(ap, uint64_t), buf, 8)
					: utoa(va_arg(ap, uint32_t), buf, 8);
				break;
			case 'x':
				intlevel == 2
					? ulltoa(va_arg(ap, uint64_t), buf, 16)
					: utoa(va_arg(ap, uint32_t), buf, 16);
				break;
			}
			if (size_override) {
				buf_idx = strlen(buf);
				if (buf_idx < width) {
					unsigned pad = width - buf_idx;
					memmove(buf + pad, buf, buf_idx + 1);
					if (zero_pad) {
						itoa(pad, width_buf, 10);
						memset(buf, '0', pad);
					} else {
						memset(buf, ' ', pad);
					}
					buf[width] = '\0';
				}
			}
			cb(buf);
			buf_idx = 0;
			fmt++;
			intlevel = 0;
			break;
		case 'c':
			fmt++;
			if (buf_idx >= sizeof(buf) - 1) {
				_flush_buf(cb, buf, &buf_idx);
			}
			buf[buf_idx] = ((char) va_arg(ap, uint32_t));
			buf_idx++;
			continue;
		case 's':
			fmt++;
			if (buf_idx > 0) {
				_flush_buf(cb, buf, &buf_idx);
			}
			cb(va_arg(ap, const char *));
			continue;
		default:
			if (buf_idx > 0) {
				_flush_buf(cb, buf, &buf_idx);
			}
			cb(fmt_begun_at);
			return;
		}
	}
	if (buf_idx > 0) {
		_flush_buf(cb, buf, &buf_idx);
	}
}

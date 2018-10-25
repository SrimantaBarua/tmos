// (C) 2018 Srimanta Barua
//
// Writing formatted text to buffer

#include <shuos/system.h>
#include <shuos/vsprintf.h> 
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#define ZEROPAD 0x01  // Pad with 0
#define SIGN    0x02  // Unsigned/signed long
#define PLUS    0x04  // Show plus
#define SPACE   0x08  // Space if plus
#define LEFT    0x10  // Left justified
#define SPECIAL 0x20  // 0x, 0, decimal etc
#define LARGE   0x40  // Use  'ABCDEF' instead of 'abcdef'

static int _skip_atoi(const char **s) {
	int ret = 0;
	while (isdigit(**s)) {
		ret = ret * 10 + (**s - '0');
		(*s)++;
	}
	return ret;
}

static char* _number(char *buf, char *end, unsigned long long num,
		     int base, int size, int precision, int flags) {
	char c, sign, tmp[66];
	const char *digits;
	int i = 0;
	static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	digits = (flags & LARGE) ? large_digits : small_digits;
	if (flags & LEFT) {
		flags &= ~ZEROPAD;
	}
	if (base < 2 || base > 36) {
		return 0;
	}
	c = (flags & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (flags & SIGN) {
		if ((signed long long) num < 0) {
			sign = '-';
			num = -(signed long long) num;
			size--;
		} else if (flags & PLUS) {
			sign = '+';
			size--;
		} else if (flags & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (flags & SPECIAL) {
		if (base == 16) {
			size -= 2;
		} else if (base == 8) {
			size -= 1;
		}
	}
	if (num == 0) {
		tmp[i++] = '0';
	} else {
		while (num) {
			tmp[i++] = digits[num % base];
			num /= base;
		}
	}
	if (i > precision) {
		precision = i;
	}
	size -= precision;
	if (!(flags & (ZEROPAD | LEFT))) {
		while (size-- > 0) {
			if (buf <= end) {
				*buf = ' ';
			}
			buf++;
		}
	}
	if (sign) {
		if (buf <= end) {
			*buf = sign;
		}
		buf++;
	}
	if (flags & SPECIAL) {
		if (base == 8) {
			if (buf <= end) {
				*buf = '0';
			}
			buf++;
		} else if (base == 16) {
			if (buf <= end) {
				*buf = '0';
			}
			buf++;
			if (buf <= end) {
				*buf = digits[33];
			}
			buf++;
		}
	}
	if (!(flags & LEFT)) {
		while (size-- > 0) {
			if (buf <= end) {
				*buf = c;
			}
			buf++;
		}
	}
	while (i < precision--) {
		if (buf <= end) {
			*buf = '0';
		}
		buf++;
	}
	while (i-- > 0) {
		if (buf <= end) {
			*buf = tmp[i];
		}
		buf++;
	}
	while (size-- > 0) {
		if (buf <= end) {
			*buf = ' ';
		}
		buf++;
	}
	return buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) {
	char *str, *end, c;
	const char *fmt_begun_at, *s;
	int flags, field_width, qualifier, precision, base, i;
	int slen;
	unsigned long long num;
	str = buf;
	end = buf + size - 1;
	if (end < buf - 1) {
		end = ((void*) -1);
		size = end - buf + 1;
	}
	for ( ; *fmt; fmt++) {
		if (*fmt != '%') {
			if (str <= end) {
				*str = *fmt;
			}
			str++;
			continue;
		}
		fmt_begun_at = fmt;
		// Get flags
		flags = 0;
		while (1) {
			fmt++;  // This also skips first '%'
			switch (*fmt) {
			case '+':
				flags |= PLUS;
				continue;
			case '-':
				flags |= LEFT;
				continue;
			case '#':
				flags |= SPECIAL;
				continue;
			case ' ':
				flags |= SPACE;
				continue;
			case '0':
				flags |= ZEROPAD;
				continue;
			}
			break;
		}
		// Get field width
		field_width = -1;
		if (isdigit(*fmt)) {
			field_width = _skip_atoi(&fmt);
		} else if (*fmt == '*') {
			fmt++;
			// Next argument is width
			field_width = va_arg(ap, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}
		// Get precision
		precision = -1;
		if (*fmt == '.') {
			fmt++;
			if (isdigit(*fmt)) {
				precision = _skip_atoi(&fmt);
			} else if (*fmt == '*') {
				fmt++;
				// Next argument is precision
				precision = va_arg(ap, int);
			}
			if (precision < 0) {
				precision = 0;
			}
		}
		// Get conversion qualifier
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z' || *fmt == 'z') {
			qualifier = *fmt;
			fmt++;
			if (qualifier == 'l' && *fmt == 'l') {
				qualifier = 'L';
				fmt++;
			}
		}
		base = 10;
		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT)) {
				while (--field_width > 0) {
					if (str <= end) {
						*str = ' ';
					}
					str++;
				}
			}
			c = (char) va_arg(ap, int);
			if (str <= end) {
				*str = c;
			}
			str++;
			while (--field_width > 0) {
				if (str <= end) {
					*str = ' ';
				}
				str++;
			}
			continue;
		case 's':
			s = va_arg(ap, const char *);
			if ((uintptr_t) s < PAGE_SIZE) {
				s = "(null)";
			}
			slen = strlen(s);
			if (!(flags & LEFT)) {
				while (slen < field_width--) {
					if (str <= end) {
						*str = ' ';
					}
					str++;
				}
			}
			for (i = 0; i < slen; i++) {
				if (str <= end) {
					*str = *s;
				}
				str++;
				s++;
			}
			continue;
		case 'p':
			if  (field_width == -1) {
				field_width = 2 * sizeof(void*);
				flags |= ZEROPAD;
			}
			str = _number(str, end, (unsigned long long) va_arg(ap, void*),
					16, field_width, precision, flags);
			continue;
		case '%':
			if (str <= end) {
				*str = '%';
			}
			str++;
			continue;
		case 'o':
			base = 8;
			break;
		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;
		case 'n': // TODO
		default:
			while (fmt_begun_at < fmt) {
				if (str <= end) {
					*str = *fmt_begun_at;
					fmt_begun_at++;
				}
				str++;
			}
			if (*fmt) {
				if (str <= end) {
					*str = *fmt;
				}
				str++;
			} else {
				fmt--;
			}
			continue;
		}
		if (qualifier == 'L') {
			num = va_arg(ap, unsigned long long);
		} else if (qualifier == 'l') {
			num = (unsigned long) va_arg(ap, unsigned long);
			if (flags & SIGN) {
				num = (signed long) num;
			}
		} else if (qualifier == 'Z' || qualifier == 'z') {
			num = va_arg(ap, size_t);
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(ap, int);
			if (flags & SIGN) {
				num = (signed short) num;
			}
		} else {
			num = (unsigned int) va_arg(ap, unsigned int);
			if (flags & SIGN) {
				num = (signed int) num;
			}
		}
		str = _number(str, end, num, base, field_width, precision, flags);
	}
	if (str <= end) {
		*str = '\0';
	} else if (size > 0) {
		*end = '\0';
	}
	return str - buf;
}

int vsprintf(char *buf, const char *fmt, va_list ap) {
	return vsnprintf(buf, SIZE_MAX, fmt, ap);
}

int sprintf(char *buf, const char *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsprintf(buf, fmt, ap);
	va_end(ap);
	return ret;
}

int snprintf(char *buf, size_t size, const char *fmt, ...) {
	int ret;
	va_list ap;
	va_start(ap, fmt);
	ret = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return ret;
}

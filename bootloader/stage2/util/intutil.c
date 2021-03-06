// (C) 2018 Srimanta Barua
//
// Utility functions for formatting

#include <intutil.h>

static const char *_HEXTABLE = "0123456789abcdef";

char* itoa(int32_t val, char *buf, uint32_t base) {
	if (!val || !base || base > 16) {
		buf[0] = '0';
		buf[1] = 0;
		return buf;
	}
	if (base != 10 || val > 0) {
		return utoa (val, buf, base);
	}
	buf[0] = '-';
	utoa (-val, buf + 1, base);
	return buf;
}

char* utoa(uint32_t val, char *buf, uint32_t base) {
	int tmp_idx = 0, buf_idx = 0;
	char tmp[32];
	if (!val || !base || base > 16) {
		buf[0] = '0';
		buf[1] = 0;
		return buf;
	}
	while (val > 0) {
		tmp[tmp_idx++] = _HEXTABLE[val % base];
		val = val / base;
	}
	tmp_idx--;
	while  (tmp_idx >= 0) {
		buf[buf_idx++] = tmp[tmp_idx--];
	}
	buf[buf_idx] = 0;
	return buf;
}

char* lltoa(int64_t val, char *buf, uint64_t base) {
	if (!val || !base || base > 16) {
		buf[0] = '0';
		buf[1] = 0;
		return buf;
	}
	if (base != 10 || val > 0) {
		return ulltoa (val, buf, base);
	}
	buf[0] = '-';
	ulltoa (-val, buf + 1, base);
	return buf;
}

char* ulltoa(uint64_t val, char *buf, uint64_t base) {
	int tmp_idx = 0, buf_idx = 0;
	char tmp[64];
	if (!val || !base || base > 16) {
		buf[0] = '0';
		buf[1] = 0;
		return buf;
	}
	while (val > 0) {
		tmp[tmp_idx++] = _HEXTABLE[val % base];
		val = val / base;
	}
	tmp_idx--;
	while  (tmp_idx >= 0) {
		buf[buf_idx++] = tmp[tmp_idx--];
	}
	buf[buf_idx] = 0;
	return buf;
}

int atoi(const char *s) {
	int sign = 1, res = 0;
	if (*s == '-') {
		sign = -1;
		s++;
	}
	while (*s) {
		res = res * 10 + (int) (*s - '0');
		s++;
	}
	return sign * res;
}

long atol(const char *s) {
	long sign = 1, res = 0;
	if (*s == '-') {
		sign = -1;
		s++;
	}
	while (*s) {
		res = res * 10 + (long) (*s - '0');
	}
	return sign * res;
}

long long atoll(const char *s) {
	long long sign = 1, res = 0;
	if (*s == '-') {
		sign = -1;
		s++;
	}
	while (*s) {
		res = res * 10 + (long long) (*s - '0');
	}
	return sign * res;
}

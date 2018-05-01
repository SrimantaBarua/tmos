// (C) 2018 Srimanta Barua
//
// Arch-neutral implementations of string.h functions

#include <string.h>
#include <stdint.h>

#ifndef __HAS_ARCH_MEMCPY__
void* memcpy(void *dest, const void *src, size_t n) {
	char *d = (char *) dest;
	const char *s = (const char *) src;
	size_t i;
	for (i = 0; i < n; i++) {
		d[i] = s[i];
	}
	return dest;
}
#endif

#ifndef __HAS_ARCH_MEMMOVE__
void* memmove(void *dest, const void *src, size_t n) {
	if ((uintptr_t) dest < (uintptr_t) src) {
		return memcpy (dest, src, n);
	}
	char *d = (char *) dest;
	const char *s = (const char *) src;
	size_t i;
	for (i = n; i > 0; i--) {
		d[i - 1] = s[i - 1];
	}
	return dest;
}
#endif

#ifndef __HAS_ARCH_MEMSET__
void* memset(void *dest, unsigned char val, size_t n) {
	unsigned char *d = (unsigned char *) dest;
	size_t i;
	for (i = 0; i < n; i++) {
		d[i] = val;
	}
	return dest;
}
#endif

#ifndef __HAS_ARCH_MEMCHR__
void* memchr(const void *dest, unsigned char val, size_t n) {
	unsigned char *d = (unsigned char *) dest;
	size_t i;
	for (i = 0; i < n; i++) {
		if (d[i] == val) {
			return (void*) (dest + i);
		}
	}
	return NULL;
}
#endif

#ifndef __HAS_ARCH_MEMCMP__
int memcmp(const void *aptr, const void *bptr, size_t n) {
	char *a = (char *) aptr;
	const char *b = (const char *) bptr;
	size_t i;
	for (i = 0; i < n; i++) {
		if (a[i] != b[i]) {
			return (int) a[i] - (int) b[i];
		}
	}
	return 0;
}
#endif

#ifndef __HAS_ARCH_STRLEN__
size_t strlen(const char *s) {
	size_t ret = 0;
	if (s) {
		while (s[ret]) {
			ret++;
		}
	}
	return ret;
}
#endif

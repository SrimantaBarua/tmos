// (C) 2018 Srimanta Barua
//
// Arch-neutral implementations of string.h functions

#include <string.h>
#include <stdint.h>
#include <ctype.h>

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
		return memcpy(dest, src, n);
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

size_t strlen(const char *s) {
	size_t ret = 0;
	if (s) {
		while (s[ret]) {
			ret++;
		}
	}
	return ret;
}

char* strcpy(char *dest, const char *src) {
	char *ret = dest;
	if (src && dest) {
		while (*src) {
			*dest++ = *src++;
		}
	}
	return ret;
}

char* strncpy(char *dest, const char *src, size_t n) {
	char *ret = dest;
	size_t i = 0;
	if (src && dest) {
		while (*src && i < n) {
			*dest++ = *src++;
			i++;
		}
	}
	return ret;
}

char* strcat(char *dest, const char *src) {
	strcpy(dest + strlen(dest), src);
	return dest;
}

char* strncat(char *dest, const char *src, size_t n) {
	size_t len = strlen(dest);
	if (len < n) {
		strncpy(dest + len, src, n - len);
	}
	return dest;
}

int strcmp(const char *a, const char *b) {
	if (a && b) {
		while (*a && *b) {
			if (*a != *b) {
				return (int) *a - (int) *b;
			}
			a++;
			b++;
		}
		return (int) *a - (int) *b;
	}
	return 0;
}

int stricmp(const char *a, const char *b) {
	char tmpa, tmpb;
	if (a && b) {
		while (*a && *b) {
			tmpa = tolower(*a);
			tmpb = tolower(*b);
			if (tmpa != tmpb) {
				return (int) tmpa - (int) tmpb;
			}
			a++;
			b++;
		}
		return (int) tolower (*a) - (int) tolower(*b);
	}
	return 0;
}

int strncmp(const char *a, const char *b, size_t n) {
	size_t i = 0;
	if (a && b) {
		while (*a && *b && i < n) {
			if (*a != *b) {
				return (int) *a - (int) *b;
			}
			a++;
			b++;
			i++;
		}
		if (i == n) {
			return 0;
		}
		return (int) *a - (int) *b;
	}
	return 0;
}

int strnicmp(const char *a, const char *b, size_t n) {
	size_t i = 0;
	char tmpa, tmpb;
	if (a && b) {
		while (*a && *b && i < n) {
			tmpa = (char) tolower(*a);
			tmpb = (char) tolower(*b);
			if (tmpa != tmpb) {
				return (int) tmpa - (int) tmpb;
			}
			a++;
			b++;
			i++;
		}
		if (i == n) {
			return 0;
		}
		return (int) tolower (*a) - (int) tolower(*b);
	}
	return 0;
}

char* strchr(const char *s, char c) {
	if (s) {
		while (*s) {
			if (*s == c) {
				return (char*) s;
			}
			s++;
		}
	}
	return NULL;
}

char* strrchr(const char *s, char c) {
	const char *optr = s;
	if (s) {
		s += strlen(s) - 1;
		while (s >= optr) {
			if (*s == c) {
				return (char*) s;
			}
			s--;
		}
	}
	return NULL;
}

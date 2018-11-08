// (C) 2018 Srimanta Barua

#include <string.h>
#include <stdint.h>
#include <ctype.h>

// Copy n bytes of memory from src to dest
void* memcpy(void *dest, const void *src, size_t n) {
	char *d = (char *) dest;
	const char *s = (const char *) src;
	size_t i;
	for (i = 0; i < n; i++) {
		d[i] = s[i];
	}
	return dest;
}

// Copy n bytes of memory from src to dest. Check for overlap
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

// Set n bytes of memory at dest to val
void* memset(void *dest, int ival, size_t n) {
	unsigned char val = (unsigned char) ival;
	unsigned char *d = (unsigned char *) dest;
	size_t i;
	for (i = 0; i < n; i++) {
		d[i] = val;
	}
	return dest;
}

// Search the first n bytes of mem for val
void* memchr(const void *dest, int ival, size_t n) {
	unsigned char val = (unsigned char) ival;
	unsigned char *d = (unsigned char *) dest;
	size_t i;
	for (i = 0; i < n; i++) {
		if (d[i] == val) {
			return (void*) (dest + i);
		}
	}
	return NULL;
}

// Compare the first n bytes at a and b, return 0 if identical. If not,
// return negative if byte from a is lower than byte from b, and positive
// otherwise
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

// Return length of null-terminated string s
size_t strlen(const char *s) {
	size_t ret = 0;
	if (s) {
		while (s[ret]) {
			ret++;
		}
	}
	return ret;
}

// Copy null-terminated string from src to dest
char* strcpy(char *dest, const char *src) {
	char *ret = dest;
	if (src && dest) {
		while (*src) {
			*dest++ = *src++;
		}
	}
	return ret;
}

// Copy upto n bytes of null-terminated string from src to dest.
// NOTE: This does not ensure that dest will be null-terminated
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

// Concatenate string src to the end of string dest
char* strcat(char *dest, const char *src) {
	strcpy(dest + strlen(dest), src);
	return dest;
}

// Concatenate string src to then end of string dest, with a maximum length
// limit of the resulting string being n. Does not ensure that the resulting
// string will be null-terminated
char* strncat(char *dest, const char *src, size_t n) {
	size_t len = strlen(dest);
	if (len < n) {
		strncpy(dest + len, src, n - len);
	}
	return dest;
}

// Compare strings a and b. Similar to memcmp
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

// Compare strings a and b. Case insensitive
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

// Compare upto the first n characters of string a and b
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

// Compare upto the first n characters of string a and b. Case insensitive
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

// Find the first occurence of c in s
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

// Find the last occurence of c in s
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

// Find the first occurence of string needle in string haystack
// Naive implementation
char* strstr(const char *haystack, const char *needle) {
	const char *h, *n;
	if (!haystack || !needle) {
		return NULL;
	}
	for ( ; *haystack; haystack++) {
		for (h = haystack, n = needle; *h && *n && *h == *n; h++, n++) { }
		if (!*n) {
			return (char*) haystack;
		}
	}
	return NULL;
}

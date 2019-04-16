// (C) 2018 Srimanta Barua
//
// A tiny subset of the functionality from the standard C library's string.h

#pragma once

#include <stddef.h>

// Copy memory without checking for overlap
void* memcpy(void *dest, const void *src, size_t n);

// Copy memory with checking for overlap
void* memmove(void *dest, const void *src, size_t n);

// Set 'n' bytes to 'val'
void* memset(void *dest, int val, size_t n);

// Compare 'n' bytes
int memcmp(const void *a, const void *b, size_t n);

// Get length of null terminated string
size_t strlen(const char *s);

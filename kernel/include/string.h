// (C) 2018 Srimanta Barua
//
// Subset of the standard C string.h

#pragma once

#include <stddef.h>

#if defined(__CFG_ARCH_x86_64__)
#include <arch/x86_64/string.h>
#endif

void* memcpy(void *dest, const void *src, size_t n);
void* memmove(void *dest, const void *src, size_t n);
void* memset(void *dest, unsigned char val, size_t n);
void* memchr(const void *mem, unsigned char val, size_t n);
int memcmp(const void *a, const void *b, size_t n);

size_t strlen(const char *s);

char* strcpy(char *dest, const char *src);
char* strncpy(char *dest, const char *src, size_t n);

char* strcat(char *dest, const char *src);
char* strncat(char *dest, const char *src, size_t n);

int strcmp(const char *a, const char *b);
int stricmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
int strnicmp(const char *a, const char *b, size_t n);

char* strchr(const char *s, char c);
char* strrchr(const char *s, char c);
char* strstr(const char *haystack, const char *needle);

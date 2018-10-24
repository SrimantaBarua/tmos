// (C) 2018 Srimanta Barua

#pragma once

#include <stddef.h>

// Copy n bytes of memory from src to dest
void* memcpy(void *dest, const void *src, size_t n);

// Copy n bytes of memory from src to dest. Check for overlap
void* memmove(void *dest, const void *src, size_t n);

// Set n bytes of memory at dest to val
void* memset(void *dest, unsigned char val, size_t n);

// Search the first n bytes of mem for val
void* memchr(const void *mem, unsigned char val, size_t n);

// Compare the first n bytes at a and b, return 0 if identical. If not,
// return negative if byte from a is lower than byte from b, and positive
// otherwise
int memcmp(const void *a, const void *b, size_t n);

// Return length of null-terminated string s
size_t strlen(const char *s);

// Copy null-terminated string from src to dest
char* strcpy(char *dest, const char *src);

// Copy upto n bytes of null-terminated string from src to dest.
// NOTE: This does not ensure that dest will be null-terminated
char* strncpy(char *dest, const char *src, size_t n);

// Concatenate string src to the end of string dest
char* strcat(char *dest, const char *src);

// Concatenate string src to then end of string dest, with a maximum length
// limit of the resulting string being n. Does not ensure that the resulting
// string will be null-terminated
char* strncat(char *dest, const char *src, size_t n);

// Compare strings a and b. Similar to memcmp
int strcmp(const char *a, const char *b);

// Compare strings a and b. Case insensitive
int stricmp(const char *a, const char *b);

// Compare upto the first n characters of string a and b
int strncmp(const char *a, const char *b, size_t n);

// Compare upto the first n characters of string a and b. Case insensitive
int strnicmp(const char *a, const char *b, size_t n);

// Find the first occurence of c in s
char* strchr(const char *s, char c);

// Find the last occurence of c in s
char* strrchr(const char *s, char c);

// Find the first occurence of string needle in string haystack
char* strstr(const char *haystack, const char *needle);

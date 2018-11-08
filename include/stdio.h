// (C) 2018 Srimanta Barua

#pragma once

#include <stdarg.h>
#include <stddef.h>

// Definition of 'whence' for fseek
#define SEEK_SET 0

// FILE stream
typedef struct file FILE;

// Standard file descriptors
extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

// Open a file stream
FILE* fopen(const char *pathname, const char *mode);

// Associate a stream with existing file descriptor, fd
FILE* fdopen(int fd, const char *mode);

// Close a stream
void fclose(FILE *stream);

// Flush a stream
void fflush(FILE *stream);

// Seek to given position in a stream
int fseek(FILE* stream, long offset, int whence);

// Get the current position in a stream
long ftell(FILE* stream);

// Binary stream read
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);

// Binary stream write
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);

// Write formatted output to stream
int fprintf(FILE* stream, const char* format, ...);

// Write formatted output  to stream, but using va_list instread of varargs
int vfprintf(FILE* stream, const char* format, va_list ap);

// Stream buffering options
void setbuf(FILE* stream, char* buf);

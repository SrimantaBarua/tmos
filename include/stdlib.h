// (C) 2018 Srimanta Barua

#pragma once

#include <stddef.h>

// Register function to be called at normal process termination, either via
// exit() or via return from program's main()
int atexit(void (*function)(void));

// Convert a string to an integer
int atoi(const char *nptr);

// Cause abnormal process termination
void abort();

//  Allocate size bytes of dynamic memory and return a pointer
void* malloc(size_t size);

// Free dynamic memory pointed to be pointer
void free(void *ptr);

// Get an environment variable
char* getenv(const char *name);

// Cause normal process termination
void __attribute__((noreturn)) exit(int status);

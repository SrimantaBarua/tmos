// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>
#include <stddef.h>


// ---- pthreads ----

// Thread attributes
typedef struct {
	void   *stack_addr; // Pointer to thread's stack
	size_t stack_size;  // Size of stack in bytes
} pthread_attr_t;

// Used to identify a pthread
typedef uint32_t pthread_t;

// Used for dynamic package initialization
typedef struct {
	int is_init;  // Is this structure initialized?
	int is_exec;  // Has the initialization routine been run
} pthread_once_t;

// Thread-specific data keys
typedef uint32_t pthread_key_t;

// Used to identify a mutex
typedef uint32_t pthread_mutex_t;

// Mutex attribute
typedef struct {
	int is_initialized;  // Has the mutex been initialized
	int type;
} pthread_mutexattr_t;

// Identify a condition variable
typedef uint32_t pthread_cond_t;

typedef struct {
	int is_initialized;  // Has thread condattr been initialized
} pthread_condattr_t;


// ---- misc. ----

// Offset into a file
typedef long off_t;

// Signed size type
typedef long ssize_t;

// Process ID
typedef int pid_t;

// User ID
typedef int uid_t;

// File create mode
typedef int mode_t;

// Time
typedef long time_t;

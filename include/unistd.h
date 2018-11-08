// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>


// 'amode' in access(). Bitwise OR-able
#define F_OK 1  // Test for existence of file
#define R_OK 2  // Test for read permission
#define W_OK 4  // Test for write permission
#define X_OK 8  // Test for execute (search) permission


// Execute path with arguments argv and environment from 'environ'. argv is
// terminated by a NULL pointer
int execv(const char *path, const char **argv);

// Execure path with arguments argv and environment envp. argv and envp are
// terminated by NULL pointers
int execve(const char *path, const char **argv, const char **envp);

// Execute file, searching in the PATH environment variable if it contains no
// slashes, with arguments argv and environment from 'environ'. argv is
// terminated by a NULL pointer
int execvp(const char *file, const char **argv);

// Get the page size in bytes
int getpagesize();

// Get the pid of the calling process
pid_t getpid(void);

// Determine accessibility of a file
int access(const char *path, int amode);

// Creates a new process by duplicating the current process
pid_t fork(void);

// Close a file descriptor
int close(int fd);

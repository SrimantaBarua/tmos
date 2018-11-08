// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>
#include <stddef.h>

// Error value
#define MAP_FAILED ((void*) -1)

// Page protection options. OR these
#define PROT_NONE  0   // Page cannot be accessed
#define PROT_EXEC  1   // Page can be executed
#define PROT_READ  2   // Page can be read from
#define PROT_WRITE 4   // Page can be writted to

// Flag options
// (Pick one)
#define MAP_PRIVATE 1  // Changed are private
#define MAP_SHARED  2  // Share changes
// (And maybe OR some of these)
#define MAP_FIXED     4  // Interpret 'addr' exactly
#define MAP_GROWSDOWN 8  // Used for stacks
#define MAP_ANONYMOUS 16 // Mapping is not backed by any file, init to 0

// Flags for msync()
// (Pick one)
#define MS_SYNC       1  // Request update, wait for it to complete
#define MS_ASYNC      2  // Schedule update, return immediately
// (Optionally can also OR this)
#define MS_INVALIDATE 4  // Invalidate other mappings of the file, so they can be updated with value just written

// Flags for mlockall(). OR these
#define MCL_CURRENT 1  // Lock all pages which are currently mapped
#define MCL_FUTURE  2  // Lock all pages which will become mapped
// (Needs one of the others too)
#define MCL_ONFAULT 4  // Lock pages when they are faulted in, either current or future


// Lock part or all of the process' virtual memory into RAM, preventing it
// from being paged to the swap are
int mlock(const void *addr, size_t len);
int mlockall(int flags);

// Unlock part or all of the process' virtual memory, so that it can be
// swapped out if required
int munlock(const void *addr, size_t len);
int munlockall();

// Create a new map in the virtual address of the process. Starting address
// is 'addr'. If 'addr' is NULL, kernel chooses the page-aligned address to map
// memory and returns a pointer. If MAP_ANONYMOUS is not specified, contents
// of file 'fd' upto length 'len' from 'offset' are mapped
void* mmap(void *addr, size_t len, int prot, int flags, int fd, off_t offset);

// Remove the mapping
int munmap(void *addr, size_t len);

// Changes the access protections of the memory pages. 
int mprotect(void *addr, size_t len, int prot);

// Flushes changes made to the copy of a file mapped into memory, back to the
// filesystem.
int msync(void *addr, size_t len, int flags);

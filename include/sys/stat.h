// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>

// File mode bits 'mode_t'
#define S_IRWXU 0700   // Read, write, execute/search by owner
#define S_IRUSR 0400   // Read permission, owner
#define S_IWUSR 0200   // Write permission, owner
#define S_IXUSR 0100   // Execute/search permission, owner

#define S_IRWXG 070    // Read, write, execute/search by group
#define S_IRGRP 040    // Read permission, group
#define S_IWGRP 020    // Write permission, group
#define S_IXGRP 010    // Execute/search permission, group

#define S_IRWXO 07     // Read, write, execute/search by others
#define S_IROTH 04     // Read permission, others
#define S_IWOTH 02     // Write permission, others
#define S_IXOTH 01     // Execute/search permission, others

#define S_ISUID 04000  // Set-user-ID on execution
#define S_ISGID 02000  // Set-group-ID on execution
#define S_ISVTX 01000  // On directories, restricted deletion flag

// Functions

// Create a new directory with name 'path'
int mkdir(const char *path, mode_t mode);

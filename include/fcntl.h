// (C) 2018 Srimanta Barua

#pragma once

#include <sys/types.h>


// 'cmd' for use with fcntl()
#define F_DUPFD          1  // Duplicate file descriptor.
#define F_DUPFD_CLOEXEC  2  // Duplicate file descriptor with the close-on-exec flag FD_CLOEXEC set.
#define F_GETFD          3  // Get file descriptor flags.
#define F_SETFD          4  // Set file descriptor flags.
#define F_GETFL          5  // Get file status flags and file access modes.
#define F_SETFL          6  // Set file status flags.
#define F_GETLK          7  // Get record locking information.
#define F_SETLK          8  // Set record locking information.
#define F_SETLKW         9  // Set record locking information; wait if blocked.
#define F_GETOWN         10 // Get process or process group ID to receive SIGURG signals.
#define F_SETOWN         11 // Set process or process group ID to receive SIGURG signals.

// fcntl() file descriptor flags
#define FD_CLOEXEC   // Close the fd upon 'exec' family function

// 'l_type' in 'struct flock'
#define F_RDLCK 1  // Shared or read lock
#define F_WRLCK 2  // Exclusive or write lock
#define F_UNLCK 3  // Unlock

// 'l_whence' in 'struct flock'
#define SEEK_SET 1 // Seek relative to start of file
#define SEEK_CUR 2 // Seek relative to current position
#define SEEK_END 3 // Seek relative to end of file

// File access modes in 'oflag' for open() or openat()
#define O_RDONLY  0 // Open for reading only.
#define O_WRONLY  1 // Open for writing only.
#define O_RDWR    2 // Open for reading and writing.
#define O_EXEC    3 // Open for execute only (non-directory files)
#define O_SEARCH  3 // Open directory for search only

// Mask for file access modes in 'oflag' for open() or openat()
#define O_ACCMODE 3

// File status flags in 'oflag' for open() or openat()
#define O_APPEND   0x04 // Set append mode.
#define O_DSYNC    0x08 // Write according to synchronized I/O data integrity completion.
#define O_NONBLOCK 0x10 // Non-blocking mode.
#define O_RSYNC    0x20 // Synchronized read I/O operations.
#define O_SYNC     0x40 // Write according to synchronized I/O file integrity completion.

// File creation flags in 'oflag' for open() or openat()
#define O_CLOEXEC   0x0080 // Set FD_CLOEXEC for the new descriptor
#define O_CREAT     0x0100 // Create file if it doesn't exist
#define O_DIRECTORY 0x0200 // Fail if not a directory.
#define O_EXCL      0x0400 // Exclusive use flag.
#define O_NOCTTY    0x0800 // Do not assign controlling terminal.
#define O_NOFOLLOW  0x1000 // Do not follow symbolic links.
#define O_TRUNC     0x2000 // Truncate flag.
#define O_TTY_INIT  0x4000 // Set termios terminal paramx to conforming

// Could be used instead of 'fd' for *at() functions
#define AT_FDCWD -2 // Use the CWD for relative paths

// 'flag' used in faccessat()
#define AT_EACCESS 1 // Check access using effective user and group ID.

// 'flag' used by fstatat(), fchmodat(), fchownat(), and utimensat()
#define AT_SYMLINK_NOFOLLOW 2 // Do not follow symbolic links.

// 'flag' used by linkat()
#define AT_SYMLINK_FOLLOW 4 // Follow symbolic link.

// 'flag' used by unlinkat()
#define AT_REMOVEDIR 8 // Remove directory instead of file.


// File lock
struct flock {
	short l_type;    // Type of lock (F_RDLCK, F_WRLCK, F_UNLCK)
	short l_whence;  // Flag for starting offset
	off_t l_start;   // Relative offset in bytes
	off_t l_len;     // Size; if 0 then until EOF
	pid_t l_pid;     // PID holding the lock. Returned with F_GETLK
};


// Open a file
int open(const char *path, int oflag, ...);

// Open a file relative to a directory
int openat(int fd, const char *path, int oflag, ...);

// File control
int fcntl(int fd, int cmd, ...);

// Create a new file or rewrite an existing one
int creat(const char *path, mode_t mode);

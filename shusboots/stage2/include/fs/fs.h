// (C) 2019 Srimanta Barua
//
// General interface for dealing with file systems

#pragma once

#include <stdint.h>
#include <stdbool.h>


// Type of file system
enum fs_type {
	FSTYP_FAT12,
};


// Declaration of a fs struct
struct fs;


// A file handle
struct f_handle {
	uint64_t size; // Total size of file
	uint64_t seek; // Current seek in file
};


// A file system backend
struct fs_backend {
	bool (*ident) (const void *ptr, uint32_t num_sectors); // Verify if backend is appropriate
	int  (*open) (struct fs *fs, const char *path);        // Open file in FS
	int  (*close) (int fd);                                // Close file
	int  (*read) (int fd, void *buf, int len);             // Read data from file
};


// A generic file system object
struct fs {
	const struct fs_backend *backend;       // File system backend
	uint64_t                start_sector;   // Start sector
	uint32_t                num_sectors;    // Total number of sectors
};


// Initialize file system struct
int fs_init(struct fs *fs, uint64_t start_sector, uint32_t num_sectors);

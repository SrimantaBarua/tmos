// (C) 2019 Srimanta Barua


#include <stddef.h>
#include <disk.h>
#include <log.h>
#include <fs/fs.h>


// Temporary store for sector data
#define FS_SECTOR_PTR  0x30000


// Known FS backends
extern const struct fs_backend fs_backend_fat12;


// Static array of backends
static const struct fs_backend *_backends[] = {
	&fs_backend_fat12,
};


// Initialize file system struct
int fs_init(struct fs *fs, uint64_t start_sector, uint32_t num_sectors) {
	unsigned i;
	if (!fs || !num_sectors) {
		log(LOG_ERR, "fs_init: Invalid fs or num_sectors\n");
		return -1;
	}
	// Read first sector to see if we can get any information
	if (disk_read_sectors(start_sector, 1, (void*) FS_SECTOR_PTR) < 0) {
		log(LOG_ERR, "fs_init: Failed to read first sector\n");
		return -1;
	}
	// Initialize struct
	fs->start_sector = start_sector;
	fs->num_sectors = num_sectors;
	// Try identifying backend
	for (i = 0; i < sizeof(_backends) / sizeof(_backends[0]); i++) {
		if (_backends[i]->ident && _backends[i]->ident((void*) FS_SECTOR_PTR, 1)) {
			// Found. Set and return
			fs->backend = _backends[i];
			return 0;
		}
	}
	// No backend identified
	fs->backend = NULL;
	log(LOG_WARN, "fs_init: No matching file system found\n");
	return -1;
}

// (C) 2019 Srimanta Barua


#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <disk.h>
#include <util.h>
#include <log.h>
#include <fs/fs.h>


#define FAT_FIRST_SECTOR_ADDR 0x30000
#define FAT_TEMPBUF_ADDR      0x31000
#define FAT_ROOTDIR_ADDR      0x32000
#define FAT_FAT0_ADDR         0x32000


// A FAT timestamp
struct fat_timestamp {
	uint16_t secs : 5;
	uint16_t mins : 6;
	uint16_t hours : 5;
} __attribute__((packed));


// A FAT date
struct fat_date {
	uint16_t day : 5;
	uint16_t month : 4;
	uint16_t year : 7;
} __attribute__((packed));


// A FAT directory entry
struct fat_dirent {
	char                 name[8];
	char                 ext[3];
	uint8_t              attrib;
	uint8_t              user_attrib;
	char                 undelete;
	struct fat_timestamp create_time;
	struct fat_date      create_date;
	struct fat_date      access_date;
	uint16_t             cluster_high;
	struct fat_timestamp modified_time;
	struct fat_date      modified_date;
	uint16_t             cluster_low;
	uint32_t             file_size;
} __attribute__((packed));


// Standard boot record in FAT systems
struct std_boot_rec {
	uint8_t  _skip[3];              // Jump instruction
	char     oem_name[8];           // OEM name in text
	uint16_t bytes_per_sector;      // Bytes per sector
	uint8_t  sectors_per_cluster;   // Sectors per cluster
	uint16_t reserved_sectors;      // Reserved sectors, including this boot record
	uint8_t  num_fats;              // Number of FATs
	uint16_t num_dirent;            // Number of root directory entries
	uint16_t total_sectors_16;      // Num sectors. If 0, val > 65535 in total_sectors_32
	uint8_t  media_type;            // Media descriptor type
	uint16_t sectors_per_fat;       // Sectors per FAT
	uint16_t sectors_per_track;     // Sectors per track
	uint16_t num_heads;             // Number of heads/sides in media
	uint32_t num_hidden_sectors;    // Number of hidden sectors
	uint32_t total_sectors_32;      // Large sector count
} __attribute__((packed));


// Extended boot record for FAT 12/16
struct ebr_12_16 {
	uint8_t  phy_drive_num;      // Drive number
	uint8_t  _rsvd;              // Flags in Windows NT. Reserved otherwise
	uint8_t  signature;          // Signature. Must be 0x28 or 0x29
	uint32_t vol_id;             // Volume ID 'serial number'
	char     vol_label[11];      // Volume label string
	char     fat_type_label[8];  // System identifier string
} __attribute__((packed));
 

// Extended boot record for FAT 32
struct ebr_32 {
	uint32_t sectors_per_fat;     // Sectors per FAT
	uint16_t flags;               // Flags
	uint8_t  fat_ver_minor;       // FAT version (minor version)
	uint8_t  fat_ver_major;       // FAT version (major version)
	uint32_t root_cluster;        // Cluster number of root directory
	uint16_t fsinfo_sector;       // Sector number of FSInfo structure
	uint16_t backup_boot_sector;  // Sector number of backup boot sector
	uint8_t  _rsvd[12];           // Reserved. Should be 0
	uint8_t  phy_drive_num;       // Drive number
	uint8_t  nt_flags;            // Flgs in Windows NT. Reserved otherwise
	uint8_t  signature;           // Signature. Must be 0x28 or 0x29
	uint32_t vol_id;              // Volume ID 'serial number'
	char     vol_label[11];       // Volume label string
	char     fat_type_label[8];   // System identifier string
} __attribute__((packed));



// Check whether a sequence of bytes is a valid FAT filesystem header
static bool _fat_ident(const void *ptr, uint32_t num_sectors, uint16_t fat_size) {
	uint32_t total_clusters;
	const struct std_boot_rec *rec = NULL;
	const struct ebr_12_16 *ebr_12_16 = NULL;
	const struct ebr_32 *ebr_32 = NULL;
	if (num_sectors < 1) {
		return false;
	}
	rec = (const struct std_boot_rec*) ptr;
	if (rec->total_sectors_16 == 0) {
		if (rec->total_sectors_32 <= 0xffff) {
			log(LOG_WARN, "fat_indent: Large sectors should be > 65535\n");
			return false;
		}
		total_clusters = rec->total_sectors_32 / rec->sectors_per_cluster;
	} else {
		total_clusters = rec->total_sectors_16 / rec->sectors_per_cluster;
	}
	if (total_clusters < 0xff5) {
		if (fat_size != 12) {
			return false;
		}
	} else if (total_clusters < 0xfff5) {
		if (fat_size != 16) {
			return false;
		}
	} else if (total_clusters < 0xffffff5) {
		if (fat_size != 32) {
			return false;
		}
	} else {
		return false;
	}
	if (fat_size < 32) {
		ebr_12_16 = (const struct ebr_12_16*) &rec[1];
		if (ebr_12_16->signature != 0x28 && ebr_12_16->signature != 0x29) {
			log(LOG_WARN, "fat_indent: Invalid FAT signature: %u\n", ebr_12_16->signature);
			return false;
		}
	} else {
		ebr_32 = (const struct ebr_32*) &rec[1];
		if (ebr_32->signature != 0x28 && ebr_32->signature != 0x29) {
			log(LOG_WARN, "fat_indent: Invalid FAT signature: %u\n", ebr_32->signature);
			return false;
		}
	}
	log(LOG_INFO, "FAT%u\n"
		"  bytes_per_sector    : %u\n"
		"  sectors_per_cluster : %u\n"
		"  reserved_sectors    : %u\n"
		"  num_fats            : %u\n"
		"  num_dirent          : %u\n"
		"  total_sectors_16    : %u\n"
		"  sectors_per_fat     : %u\n"
		"  total_sectors_32    : %u\n"
		, fat_size, rec->bytes_per_sector, rec->sectors_per_cluster, rec->reserved_sectors, rec->num_fats,
		rec->num_dirent, rec->total_sectors_16, rec->sectors_per_fat, rec->total_sectors_32);
	return true;
}


// Identify if a sequence of bytes is a FAT12 file system
static bool _fat12_ident(const void *ptr, uint32_t num_sectors) {
	return _fat_ident(ptr, num_sectors, 12);
}


// Identify if a sequence of bytes is a FAT16 file system
static bool _fat16_ident(const void *ptr, uint32_t num_sectors) {
	return _fat_ident(ptr, num_sectors, 16);
}


// Identify if a sequence of bytes is a FAT32 file system
static bool _fat32_ident(const void *ptr, uint32_t num_sectors) {
	return _fat_ident(ptr, num_sectors, 32);
}


// List FAT directory
static void _list_dir(const struct fat_dirent *d, uint16_t num_ent) {
	uint16_t i, j;
	bool no_dirent;
	log(LOG_INFO, "List directory: /:\n");
	__log_without_typ("  Filename        Created                Modified              Size\n");
	for (i = 0; i < num_ent; i++) {
		no_dirent = false;
		for (j = 0; j < 8; j++) {
			if (d[i].name[j] == '\0') {
				no_dirent = true;
				break;
			}
		}
		for (j = 0; j < 3; j++) {
			if (d[i].ext[j] == '\0') {
				no_dirent = true;
				break;
			}
		}
		if (no_dirent) {
			continue;
		}
		__log_without_typ("  %.8s.%.3s    %4u-%02u-%02u %02u:%02u:%02u    %4u-%02u-%02u"
				"%02u:%02u:%02u    %u\n", d[i].name, d[i].ext,
				d[i].create_date.year + 1980, d[i].create_date.month,
				d[i].create_date.day, d[i].create_time.hours,
				d[i].create_time.mins, d[i].create_time.secs << 1,
				d[i].modified_date.year + 1980, d[i].modified_date.month,
				d[i].modified_date.day, d[i].modified_time.hours,
				d[i].modified_time.mins, d[i].modified_time.secs << 1,
				d[i].file_size);
	}
}


// Search for a directory entry. Return true if found and fill first cluster and size. Otherwise
// return false
static bool _find_file(const struct fat_dirent *d, uint16_t num_ent, const char *name,
		uint32_t *first_cluster, uint32_t *file_size) {
	bool is_file;
	uint16_t i, j, dotidx, nlen;
	// Find index where name ends and extension begins
	for (dotidx = 0; name[dotidx] && name[dotidx] != '.'; dotidx++);
	if (dotidx > 8) {
		return false;
	}
	// Find length of name
	for (nlen = dotidx; name[nlen]; nlen++);
	if (nlen - dotidx > 4) {
		return false;
	}
	// Go over each directory entry
	for (i = 0; i < num_ent; i++) {
		is_file = true;
		// Compare name
		for (j = 0; j < dotidx; j++) {
			if (toupper(name[j]) != d[i].name[j]) {
				is_file = false;
				break;
			}
		}
		if (j == 0) {
			continue;
		}
		for ( ; j < 8; j++) {
			if (d[i].name[j] != ' ') {
				is_file = false;
				break;
			}
		}
		if (!is_file) {
			continue;
		}
		// Compare extension
		j = 0;
		if (dotidx != nlen) {
			for ( ; j < 3 && j < nlen - dotidx - 1; j++) {
				if (toupper(name[j + dotidx + 1]) != d[i].ext[j]) {
					is_file = false;
					break;
				}
			}
		}
		for ( ; j < 3; j++) {
			if (d[i].name[j] != ' ') {
				is_file = false;
				break;
			}
		}
		if (!is_file) {
			continue;
		}
		// Found file
		*file_size = d[i].file_size;
		*first_cluster = (((uint32_t) d[i].cluster_high) << 16) | d[i].cluster_low;
		return true;
	}
	return false;
}


// Read file
static int _fat12_read(struct fs *fs, const char *path, void *buf, int len) {
	int read_count = 0;
	const struct std_boot_rec *rec;
	const struct fat_dirent *dirent;
	const char *name;
	uint16_t fat_num_sectors, root_num_sectors, read_sectors, next_cluster;
	uint32_t cluster, file_size, fat_sector, root_sector, file_sector, first_data_sector;
	const uint16_t *fat;
	if (len < 0) {
		return -1;
	}
	if (len == 0) {
		return 0;
	}
	// Load boot sector
	if (disk_read_sectors(fs->start_sector, 1, (void*) FAT_FIRST_SECTOR_ADDR) < 0) {
		log(LOG_ERR, "fat12_read: Failed to read sector: 0x%llx\n", fs->start_sector);
		return -1;
	}
	rec = (const struct std_boot_rec*) FAT_FIRST_SECTOR_ADDR;
	// Load root directory
	root_num_sectors = ((rec->num_dirent << 5) + rec->bytes_per_sector - 1) / rec->bytes_per_sector;
	root_sector = (rec->num_fats * rec->sectors_per_fat) + rec->reserved_sectors + fs->start_sector;
	if (disk_read_sectors(root_sector, root_num_sectors, (void*) FAT_ROOTDIR_ADDR) < 0) {
		log(LOG_ERR, "fat12_read: Failed to read root directory\n");
		return -1;
	}
	// List root directory
	dirent = (const struct fat_dirent*) FAT_ROOTDIR_ADDR;
	_list_dir(dirent, rec->num_dirent);
	// Search for file
	name = path;
	while (*name == '/') {
		name++;
	}
	if (!_find_file(dirent, rec->num_dirent, name, &cluster, &file_size)) {
		return -1;
	}
	// Load FAT
	fat_num_sectors = rec->sectors_per_fat * rec->num_fats;
	fat_sector = rec->reserved_sectors + fs->start_sector;
	if (disk_read_sectors(fat_sector, fat_num_sectors, (void*) FAT_FAT0_ADDR) < 0) {
		log(LOG_ERR, "fat12_read: Failed to read FAT\n");
		return -1;
	}
	fat = (const uint16_t*) FAT_FAT0_ADDR;
	// Read first sector of file
	first_data_sector = root_sector + root_num_sectors;
	file_sector = (cluster - 2) * rec->sectors_per_cluster + first_data_sector;
	read_sectors =  0;
	while (1) {
		if (disk_read_sectors(file_sector, 1, (void*) FAT_TEMPBUF_ADDR) < 0) {
			log(LOG_ERR, "fat12_read: Failed to read file\n");
			return -1;
		}
		// Copy read contents
		// If we're at end of buffer length, return
		if (len < rec->bytes_per_sector) {
			if (file_size < (uint32_t) len) {
				memcpy(buf, (void*) FAT_TEMPBUF_ADDR, file_size);
				((char*) buf)[file_size] = '\0';
				return read_count + file_size + 1;
			}
			memcpy(buf, (void*) FAT_TEMPBUF_ADDR, len);
			return read_count + len;
		}
		// If we're at end of file, return
		if (file_size < rec->bytes_per_sector) {
			memcpy(buf, (void*) FAT_TEMPBUF_ADDR, file_size);
			((char*) buf)[file_size] = '\0';
			return read_count + file_size + 1;
		}
		// Otherwise just copy
		memcpy(buf, (void*) FAT_TEMPBUF_ADDR, rec->bytes_per_sector);
		len -= rec->bytes_per_sector;
		file_size -= rec->bytes_per_sector;
		buf += rec->bytes_per_sector;
		read_count += rec->bytes_per_sector;
		// Move to next sector
		read_sectors++;
		file_sector++;
		// If we're at end of cluster, move to next cluster
		if (read_sectors == rec->sectors_per_cluster) {
			next_cluster = fat[cluster * 3 / 2];
			if ((cluster & 1)) {
				// Odd cluster
				cluster = next_cluster >> 4;
			} else {
				// Even cluster
				cluster = next_cluster & 0xfff;
			}
			file_sector = (cluster - 2) * rec->sectors_per_cluster + first_data_sector;
			read_sectors = 0;
		}
	}
}


// FAT12 backend
const struct fs_backend fs_backend_fat12 = {
	.ident = _fat12_ident,
	.read = _fat12_read,
};

// FAT16 backend
const struct fs_backend fs_backend_fat16 = {
	.ident = _fat16_ident,
	.read = NULL
};

// FAT32 backend
const struct fs_backend fs_backend_fat32 = {
	.ident = _fat32_ident,
	.read = NULL
};

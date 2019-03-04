// (C) 2019 Srimanta Barua


#include <stdbool.h>
#include <stddef.h>
#include <disk.h>
#include <log.h>
#include <fs/fs.h>


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
	if (total_clusters < 4085) {
		if (fat_size != 12) {
			return false;
		}
	} else if (total_clusters < 65525) {
		if (fat_size != 16) {
			return false;
		}
	} else {
		if (fat_size != 32) {
			return false;
		}
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
	vlog("FAT%u:\n"
		"{\n"
		"  oem_name            : %.8s\n"
		"  bytes_per_sector    : %u\n"
		"  sectors_per_cluster : %u\n"
		"  reserved_sectors    : %u\n"
		"  num_fats            : %u\n"
		"  num_dir_entries     : %u\n"
		"  total_sectors_16    : %u\n"
		"  media_type          : %u\n"
		"  sectors_per_fat     : %u\n"
		"  sectors_per_track   : %u\n"
		"  num_heads           : %u\n"
		"  num_hidden_sectors  : %u\n"
		"  total_sectors_32    : %u\n",
		fat_size, rec->oem_name, rec->bytes_per_sector, rec->sectors_per_cluster,
		rec->reserved_sectors, rec->num_fats, rec->num_dirent, rec->total_sectors_16,
		rec->media_type, rec->sectors_per_fat, rec->sectors_per_track, rec->num_heads,
		rec->num_hidden_sectors, rec->total_sectors_32);
	if (fat_size < 32) {
		vlog("  volume_label        : %.11s\n"
			"  fat_type_label      : %.8s\n"
			"}\n", ebr_12_16->vol_label, ebr_12_16->fat_type_label);
	} else {
		vlog("  fat_version         : %u.%u\n"
			"  volume_label        : %.11s\n"
			"  fat_type_label      : %.8s\n"
			"}\n", ebr_32->fat_ver_major, ebr_32->fat_ver_minor, ebr_32->vol_label,
			ebr_32->fat_type_label);
	}
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


// FAT12 backend
const struct fs_backend fs_backend_fat12 = {
	.ident = _fat12_ident,
	.open = NULL,
	.close = NULL,
	.read = NULL
};

// FAT16 backend
const struct fs_backend fs_backend_fat16 = {
	.ident = _fat16_ident,
	.open = NULL,
	.close = NULL,
	.read = NULL
};

// FAT32 backend
const struct fs_backend fs_backend_fat32 = {
	.ident = _fat32_ident,
	.open = NULL,
	.close = NULL,
	.read = NULL
};

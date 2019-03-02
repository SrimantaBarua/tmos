// (C) 2019 Srimanta Barua


#include <stddef.h>
#include <disk.h>
#include <log.h>
#include <fs/fs.h>


// Standard boot record in FAT systems
struct std_boot_rec {
	uint8_t  skip[3];
	char     oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t  sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t  num_fats;
	uint16_t num_dirent;
	uint16_t total_sectors;
	uint8_t  media_type;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t num_heads;
	uint32_t num_hidden_sectors;
	uint32_t large_sectors;
} __attribute__((packed));


 // Extended boot record for FAT 12/16
struct ebr_12_16 {
	uint8_t  phy_drive_num;
	uint8_t  rsvd;
	uint8_t  signature;
	uint32_t id;
	char     vol_label[11];
} __attribute__((packed));
 

 // Extended boot record for FAT 32


// FAT12 backend
const struct fs_backend fs_backend_fat12 = {
	.ident = NULL,
	.open = NULL,
	.close = NULL,
	.read = NULL
};

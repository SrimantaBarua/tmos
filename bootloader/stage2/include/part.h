// (C) 2019 Srimanta Barua
//
// Interface for partition information?

#pragma once


#include <stdint.h>


// A CHS address
struct mbr_chs {
	uint8_t head;
	uint16_t sector : 6;
	uint16_t cylinder : 10;
} __attribute__((packed));


// Structure of an MBR partition
struct mbr_part {
	uint8_t status;          // Status of physical drive (bit 7 for bootable)
	struct mbr_chs first;    // CHS of first absolute sector
	uint8_t type;            // Partition type
	struct mbr_chs last;     // CHS of last absolute sector
	uint32_t lba_first;      // LBA of first absolute sector
	uint32_t num_sectors;    // Total number of sectors
} __attribute__ ((packed));

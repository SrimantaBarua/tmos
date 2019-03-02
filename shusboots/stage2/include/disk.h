// (C) 2019 Srimanta Barua
//
// Interface for disk I/O (uses BIOS interrupt 0x13 underneath)

#pragma once

#include <stdint.h>


// Initialize the disk I/O subsystem
int disk_init(uint8_t boot_drive);

// Read n sectors, starting from s0, to dest. Return 0 on success, -1 on failure 
int disk_read_sectors(uint64_t s0, uint16_t n, void *dest);

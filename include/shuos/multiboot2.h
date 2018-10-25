// (C) 2018 Srimanta Barua

#pragma once

#include <shuos/system.h>

// The multiboot2 table's fixed header
struct mb2_table {
	uint32_t size;
	uint32_t _rsvd;
};

// Global pointer to multiboot table. Doesn't not need locking because it is only set once
extern const struct mb2_table *MB2TAB;

// Type of multiboot table tags (TODO: Support more)
#define MB2_TAG_TYPE_CMDLINE    1   // Boot command line
#define MB2_TAG_TYPE_BOOTLOADER 2   // Bootloader name
#define MB2_TAG_TYPE_MODULES    3   // Loaded modules
#define MB2_TAG_TYPE_MMAP       6   // Memory map
#define MB2_TAG_TYPE_VBE        7   // VBE info
#define MB2_TAG_TYPE_FRAMEBUF   8   // Framebuffer
#define MB2_TAG_TYPE_ELF        9   // ELF-sections tag
#define MB2_TAG_TYPE_END        0   // Tag which terminates the list

// Generic tag header
struct mb2_tag {
	uint32_t type;
	uint32_t size;
};

// Tag for boot modules loaded alongside the kernel
struct mb2_tag_module {
	uint32_t type;   // = 3
	uint32_t size;
	uint32_t start;  // Starting PHYSICAL address
	uint32_t end;    // Ending PHYSICAL address
};

// Memory map tag
struct mb2_tag_mmap {
	uint32_t type;   // = 6
	uint32_t size;
	uint32_t entsz;  // Size of one entry
	uint32_t ver;    // Version
};

// Type of memory map region
#define MB2_MMAP_REGION_AVAIL    1
#define MB2_MMAP_REGION_RSVD     2
#define MB2_MMAP_REGION_ACPI     3
#define MB2_MMAP_REGION_ACPI_NVS 4

// Multiboot memory region
struct mb2_mmap_region {
	uint64_t start;
	uint64_t len;
	uint32_t type;
	uint32_t _rsvd;
};

// ELF sections tag
struct mb2_tag_elf {
	uint32_t type;   // = 9
	uint32_t size;
	uint32_t num;    // Number of sections
	uint32_t entsz;  // Size of one section entry
	uint32_t shndx;  // String table index for section header names
};

// Boot command-line
// NOTE: Take care not to use sizeof ()
struct mb2_cmdline {
	uint32_t type;   // = 1
	uint32_t size;   // Size of whole tag
	char cmdline[1]; // Variable length string
};

// Bootloader name
// NOTE: Take care not to use sizeof ()
struct mb2_bootloader {
	uint32_t type;   // = 2
	uint32_t size;   // Size of whole tag
	char name[1];    // Variable length string
};

// Load multiboot2 table from address after checking if valid. -1 on error, 0 on success
int mb2_table_load(vaddr_t addr);

// Get a pointer to a multiboot2 tag of the given type
const struct mb2_tag* mb2_get_tag(uint32_t type);

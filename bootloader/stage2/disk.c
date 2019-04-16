#include <log.h>
#include <disk.h>
#include <real.h>
#include <string.h>


extern void real_disk_check_int13_ext();
extern void real_disk_read_sectors();
extern void real_disk_read_ext_params();


// A disk address packet
struct disk_address_packet {
	uint8_t  size;         // Size of DAP (16 bytes)
	uint8_t  _rsvd;        // Reserved (should be 0)
	uint16_t num_sectors;  // Number of sectors to read. Some Phoenix BIOSes limit to  max 127
	uint16_t off;          // Offset part of pointer to memory buffer
	uint16_t seg;          // Segment part of pointer to memory buffer
	uint64_t start_sector; // Absolute number of start sector (first sector of disk is 0)
} __attribute__ ((packed));


// Result buffer to store the results of reading disk parameters
struct result_buffer {
	uint16_t size;                // Size of result buffer (30 bytes)
	uint16_t flags;               // Information flags
	uint32_t cylinders;           // Physical number of cylinders
	uint32_t heads;               // Physical number of heads
	uint32_t sectors_per_track;   // Physical number of sectors per track
	uint64_t sectors;             // Absolute number of sectors
	uint16_t bytes_per_sector;    // Bytes per sector
	uint32_t edd_ptr;             // Optional pointer to enhanced disk drive params
} __attribute__((packed));


// Static DAP
static struct disk_address_packet _dap = { 0 };

// Static result buffer. Stores disk information
static struct result_buffer _disk_params = { 0 };

// Static copy of boot drive
static uint8_t _boot_drive = 0;


// Initialize the disk I/O subsystem
int disk_init(uint8_t boot_drive) {
	struct real_ptr ptr;
	// Check if int 0x13 extensions are supported
	if (!real_call(real_disk_check_int13_ext, 1, boot_drive)) {
		log(LOG_ERR, "disk_init: int 13h extensions not supported for "
			"drive: %u\n", boot_drive);
		return -1;
	}
	// Read extended disk parameters
	memset(&_disk_params, 0, sizeof(_disk_params));
	_disk_params.size = sizeof(_disk_params);
	real_ptr_from_linear((uint32_t) &_disk_params, ptr);
	if (!real_call(real_disk_read_ext_params, 3, boot_drive, ptr.off, ptr.seg)) {
		log(LOG_ERR, "disk_init: Could not read extended params for "
				"drive: %u\n", boot_drive);
		return -1;
	}
	// Store boot drive
	_boot_drive = boot_drive;
	// Print disk info
	log(LOG_INFO, "Disk information:\n"
		"{\n"
		"    size        : 0x%x\n"
		"    flags       : 0x%x\n"
		"    cylinders   : 0x%x\n"
		"    heads       : 0x%x\n"
		"    sec_per_trk : 0x%x\n"
		"    sectors     : 0x%x\n"
		"    b_per_sec   : 0x%x\n"
		"    edd_ptr     : 0x%x\n"
		"}\n", _disk_params.size, _disk_params.flags, _disk_params.cylinders,
		_disk_params.heads, _disk_params.sectors_per_track, _disk_params.sectors,
		_disk_params.bytes_per_sector, _disk_params.edd_ptr);
	return 0;
}


// Read n sectors, starting from s0, to dest. Return 0 on success, -1 on failure 
int disk_read_sectors(uint64_t s0, uint16_t n, void *dest) {
	struct real_ptr memptr, dapptr;
	if (!n) {
		return 0;
	}
	real_ptr_from_linear((uint32_t) dest, memptr);
	real_ptr_from_linear((uint32_t) &_dap, dapptr);
	// Read sectors
	memset(&_dap, 0, sizeof(_dap));
	_dap.size = sizeof(_dap);
	_dap.num_sectors = n;
	_dap.off = memptr.off;
	_dap.seg = memptr.seg;
	_dap.start_sector = s0;
	if (!real_call(real_disk_read_sectors, 3, _boot_drive, dapptr.off, dapptr.seg)) {
		log(LOG_ERR, "disk_read_sectos: Could not read sectors from"
			" drive: %u\n", _boot_drive);
		return -1;
	}
	return 0;
}


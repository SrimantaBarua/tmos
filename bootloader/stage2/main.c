// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <serial.h>
#include <log.h>
#include <mem.h>

void main() {
	// Initialize the serial port interface
	serial_init ();
	union region *regions = 0;
	uint32_t num_regions = mem_load_regions (0x10000, &regions);

	volatile uint32_t *ptr = 0xb8000;
	*ptr = 0x2f4b2f4f;
	PANIC ("Test panic: %d\n", 3);
	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

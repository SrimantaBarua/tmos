// (C) 2018 Srimanta Barua
//
// The beginning of C code. Perform arch-specific initialization and hand off the the arch-neutral
// kernel

#include <stdint.h>
#include <serial.h>

// This is called if the system is booted by a multiboot2-compliant
// bootloader. Perform initialization and set up state to the point where we can hand off to
// common kernel code
void kinit_multiboot2(uint64_t pointer) {
	// Initialize serial communication
	serial_init ();
	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	while (1);
}

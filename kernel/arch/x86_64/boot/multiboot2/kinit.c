// (C) 2018 Srimanta Barua
//
// The beginning of C code. This is called if the system is booted by a multiboot2-compliant
// bootloader. Perform initialization and set up state to the point where we can hand off to
// common kernel code

#include <stdint.h>

void kinit_multiboot2(uint64_t pointer) {
	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	while (1);
}

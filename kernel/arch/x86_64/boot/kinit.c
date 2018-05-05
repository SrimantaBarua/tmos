// (C) 2018 Srimanta Barua
//
// The beginning of C code. Perform arch-specific initialization and hand off the the arch-neutral
// kernel

#include <stdint.h>
#include <serial.h>
#include <klog.h>
#include <arch/x86_64/idt.h>

// This is called if the system is booted by a multiboot2-compliant
// bootloader. Perform initialization and set up state to the point where we can hand off to
// common kernel code
void kinit_multiboot2(uint64_t pointer) {
	// Initialize serial communication
	serial_init ();

	// Initialize and enable interrupts
	idt_init ();
	idt_enable_int ();

	// Log that we are booted
	klog ("Booting ShuOS..\n");

	// Generate fake interrupt
	__asm__ __volatile__ ("int 5" : : : );

	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	__asm__ __volatile__ ("1: hlt; jmp 1" : : : "memory");
}

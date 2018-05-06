// (C) 2018 Srimanta Barua
//
// The beginning of C code. Perform arch-specific initialization and hand off the the arch-neutral
// kernel

#include <stdint.h>
#include <serial.h>
#include <multiboot2.h>
#include <memory.h>
#include <klog.h>
#include <arch/x86_64/idt.h>

// The kernel's memory map
static struct mmap _KMMAP = { 0 };

// This is called if the system is booted by a multiboot2-compliant
// bootloader. Perform initialization and set up state to the point where we can hand off to
// common kernel code
void kinit_multiboot2(vaddr_t pointer) {
	// Initialize serial communication
	serial_init ();

	// Load multiboot2 information table
	if (mb2_table_load (pointer) < 0) {
		klog ("Failed to load multiboot2 table\n");
		crash_and_burn ();
	}

	// Parse multiboot2 memory map into our own memory map
	mem_load_mb2_mmap (&_KMMAP);

	// Initialize and enable interrupts
	idt_init ();
	idt_enable_int ();

	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	crash_and_burn ();
}

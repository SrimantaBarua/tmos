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
	uint32_t first = UINT32_MAX, tot, i;

	// Initialize serial communication
	serial_init ();

	// Load multiboot2 information table
	if (mb2_table_load (pointer) < 0) {
		klog ("Failed to load multiboot2 table\n");
		crash_and_burn ();
	}

	// Parse multiboot2 memory map into our own memory map
	mem_load_mb2_mmap (&_KMMAP);
	// Mark region for kernel
	mmap_insert_region (&_KMMAP, PAGE_ALGN_DOWN (KRNL_PHYS_START),
			PAGE_ALGN_UP (KRNL_PHYS_END), REGION_TYPE_KERNEL);
	// Split at 16MB mark, with low memory for DMA allocator
	mmap_split_at (&_KMMAP, 0x1000000);

	// Print memory map
	mmap_print (&_KMMAP);

	// Initialize low memory allocator
	for (tot = 0; tot < MMAP_MAX_NUM_ENTRIES; tot++) {
		if (REGION_TYPE (_KMMAP.r[tot]) == REGION_TYPE_AVAIL && first == UINT32_MAX) {
			first = tot - 1;
		}
		if (!REGION_START (_KMMAP.r[tot])) {
			tot++;
			break;
		}
		if (REGION_START (_KMMAP.r[tot]) == 0x1000000) {
			i = tot;
		}
	}

	BM_PMMGR.init (&_KMMAP.r[first], i + 1 - first);
	for (i = 0; i < 10; i++) {
		klog ("alloc() -> 0x%p\n", BM_PMMGR.alloc ());
	}
	BM_PMMGR.free (0x3000);
	klog ("free(0x3000)\nalloc() -> 0x%p\n", BM_PMMGR.alloc ());
	klog ("alloc() -> 0x%p\n", BM_PMMGR.alloc ());

	// Initialize and enable interrupts
	idt_init ();
	idt_enable_int ();

	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	crash_and_burn ();
}

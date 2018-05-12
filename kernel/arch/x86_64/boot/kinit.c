// (C) 2018 Srimanta Barua
//
// The beginning of C code. Perform arch-specific initialization and hand off the the arch-neutral
// kernel

#include <stdint.h>
#include <serial.h>
#include <multiboot2.h>
#include <memory.h>
#include <elf.h>
#include <arch/x86_64/memory.h>
#include <klog.h>
#include <arch/x86_64/idt.h>
#include <arch/x86_64/gdt.h>

// Guard page (defined in entry.asm)
extern int __guard_page__;

// The kernel's memory map
static struct mmap _KMMAP = { 0 };

// Callback for remapping the kernel in case we're loaded by multiboot2
static void _remap_cb_multiboot2() {
	const struct mb2_tag_elf *elftag;
	const struct elf64_shdr *shdr, *end;
	uint64_t i, n, flags;
	// Map kernel sections
	ASSERT (elftag = (const struct mb2_tag_elf*) mb2_get_tag (MB2_TAG_TYPE_ELF));
	end = (struct elf64_shdr*) ((void*) elftag + elftag->size);
	shdr = (struct elf64_shdr*) ((void*) elftag + sizeof (struct mb2_tag_elf));
	while (shdr < end) {
		// If section is below 0xffffffff80000000, then it is part of bootstrap. Discard
		if (shdr->sh_addr < KRNL_VBASE) {
			shdr++;
			continue;
		}
		// If section is not allocated, discard
		if (!(shdr->sh_flags & SHF_ALLOC)) {
			shdr++;
			continue;
		}
		// Check if page-aligned
		ASSERT (!(shdr->sh_addr & (PAGE_SIZE - 1)) && !(shdr->sh_size & (PAGE_SIZE - 1)));
		// Map section
		flags = PTE_FLG_PRESENT;
		if (shdr->sh_flags & SHF_WRITE) {
			flags |= PTE_FLG_WRITABLE;
		}
		if (!(shdr->sh_flags & SHF_EXEC)) {
			flags |= PTE_FLG_NO_EXEC;
		}
		vmm_map_to (shdr->sh_addr, shdr->sh_addr - KRNL_VBASE, shdr->sh_size >> PAGE_SIZE_SHIFT, flags);
		klog ("{ SECTION: addr: %#llx, flags: %#llx, len: %#llx }\n",
		      shdr->sh_addr, shdr->sh_flags, shdr->sh_size);
		shdr++;

	}
	// Identity map multiboot2 table (since we're not done with it yet)
	for (i = 0; REGION_START (_KMMAP.r[i]); i++) {
		if (REGION_TYPE (_KMMAP.r[i]) == REGION_TYPE_MULTIBOOT2) {
			// Found
			ASSERT (i > 0);
			n = (REGION_START (_KMMAP.r[i - 1]) - REGION_START (_KMMAP.r[i])) >> PAGE_SIZE_SHIFT;
			vmm_map_to (REGION_START (_KMMAP.r[i]), REGION_START (_KMMAP.r[i]), n, PTE_FLG_PRESENT);
			break;
		}
	}
	// Unmap the guard page
	vmm_unmap ((vaddr_t) &__guard_page__, 1);
}

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

	// Print memory map
	mmap_print (&_KMMAP);

	// Initialize and enable interrupts
	gdt_init ();
	idt_init ();

	// Initialize memory allocator
	for (tot = 0; tot < MMAP_MAX_NUM_ENTRIES; tot++) {
		if (REGION_TYPE (_KMMAP.r[tot]) == REGION_TYPE_AVAIL && first == UINT32_MAX) {
			first = tot - 1;
		}
		if (!REGION_START (_KMMAP.r[tot])) {
			tot++;
			break;
		}
	}
	BM_PMMGR.init (&_KMMAP.r[first], tot, 0x1000000, PADDR_ALGN_MASK);

	// Alloc test
	for (i = 0; i < 10; i++) {
		klog ("alloc() -> 0x%p\n", BM_PMMGR.alloc ());
	}
	BM_PMMGR.free (0x1003000);
	klog ("free(0x1003000)\nalloc() -> 0x%p\n", BM_PMMGR.alloc ());
	klog ("alloc() -> 0x%p\n", BM_PMMGR.alloc ());

	// Initialize memory management
	mem_init (&BM_PMMGR, _remap_cb_multiboot2);

	// Print current page table structure
	vmm_print_ptable ();

	vmm_map_to (0xb8000, 0xb8000, 1, PTE_FLG_PRESENT | PTE_FLG_WRITABLE);
	uint64_t *ptr = (uint64_t*) 0xb8000;
	*ptr = 0x2f592f412f4b2f4f;
	crash_and_burn ();
}

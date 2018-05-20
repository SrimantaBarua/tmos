// (C) 2018 Srimanta Barua

#include <system.h>
#include <arch/x86_64/gdt.h>

// Buffer for GDT segments
static uint64_t _buf[8] = { 0 };

// Index of next GDT segment
static uint64_t _next;

// GDTR
static struct {
	uint16_t limit;
	uint64_t base;
} PACKED _gdtr = {
	.limit = 0,
	.base = 0,
};

// Set CS
extern void set_cs(uint16_t cs);

// Load GDTR
static void _lgdt(uint64_t gdtr) {
	__asm__ __volatile__ ("lgdt [rdi];" : : "D"(gdtr) : "memory");
}

// Initialize the GDt with default segments
void gdt_init() {
	uint16_t cs, ds;
	_next = 1;
	// Add kernel code segment
	cs = gdt_add_user_seg (KRNL_CODE_SEGMENT);
	// Add kernel data segment
	ds = gdt_add_user_seg (KRNL_DATA_SEGMENT);
	// Load GDTR
	_gdtr.limit = (_next << 3) - 1;
	_gdtr.base = (uint64_t) _buf;
	_lgdt ((uint64_t) &_gdtr);
	// Set CS
	set_cs (cs);
	// Set DS
	__asm__ __volatile__ ("mov ds, ax; mov ss, ax;" : : "a"(ds) : "memory");
}

// Add a user segment
uint16_t gdt_add_user_seg(uint64_t seg) {
	uint16_t sel = (uint16_t) _next << 3;
	_buf[_next] = seg;
	_next++;
	return sel;
}

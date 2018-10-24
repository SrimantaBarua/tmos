// (C) 2018 Srimanta Barua

#include <system.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/tss.h>

// Buffer for GDT segments
static uint64_t _buf[5 + (2 * __MAX_NUM_TSS__)] = { 0 };

// Index of next GDT segment
static uint64_t _next;

// GDTR
static struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) _gdtr = {
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
	uint32_t i;
	uint16_t cs, ds;
	struct tss *tss;
	_next = 1;
	// Add kernel code segment
	cs = gdt_add_seg(KRNL_CODE_SEGMENT);
	// Add kernel data segment
	ds = gdt_add_seg(KRNL_DATA_SEGMENT);
	// Add user code segment
	gdt_add_seg(USER_CODE_SEGMENT);
	// Add user data segment
	gdt_add_seg(USER_DATA_SEGMENT);
	// For each core, add TSS
	for (i = 0; i < __MAX_NUM_TSS__; i++) {
		tss = tss_get_n(i);
		tss->ioperm_off = sizeof(struct tss);
		gdt_add_tss(tss);
	}
	// Load GDTR
	_gdtr.limit = (_next << 3) - 1;
	_gdtr.base = (uint64_t) _buf;
	_lgdt((uint64_t) &_gdtr);
	// Set CS
	set_cs(cs);
	// Set DS
	__asm__ __volatile__ ("mov ds, ax; mov ss, ax;" : : "a"(ds) : "memory");
}

// Add a user segment
uint16_t gdt_add_seg(uint64_t seg) {
	uint16_t sel = (uint16_t) _next << 3;
	_buf[_next] = seg;
	_next++;
	return sel;
}

// Add a TSS
uint16_t gdt_add_tss(const void *tss) {
	uint16_t sel = (uint16_t) _next << 3;
	_buf[_next] = (uint64_t) sizeof(struct tss) & 0xffff;
	_buf[_next] |= (((uint64_t) tss) & 0xffffff) << 16;
	_buf[_next] |= (uint64_t) 0x89 << 40;
	_buf[_next] |= ((((uint64_t) tss) >> 24) & 0xff) << 56;
	_buf[_next + 1] = (((uint64_t) tss) >> 32) & 0xffffffff;
	_next += 2;
	return sel;
}

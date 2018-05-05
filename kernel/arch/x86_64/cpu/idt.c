// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <stddef.h>
#include <arch/x86_64/idt.h>
#include <klog.h>

// The structure of an IDT entry
struct idt_entry {
	uint16_t base_low;
	uint16_t seg;

	uint8_t  ist      : 3;
	uint8_t  _rsvd0_0 : 5;

	uint8_t  gate_type : 4;
	uint8_t  storage   : 1;
	uint8_t  dpl       : 2;
	uint8_t  present   : 1;

	uint16_t base_mid;
	uint32_t base_high;

	uint32_t _rsvd0_1;
} __attribute ((packed));

// Structure of an IDT pointer
struct idt_ptr {
	uint16_t limit;
	uint64_t base;
} __attribute ((packed));


// TODO: This should probably be locked..
static struct idt_entry _IDT[256] = { 0 };
static struct idt_ptr _IDTR = { 0 };
static const struct idt_entry _IDT_ZERO = { 0 };

// Default exception handlers
extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();

// Load the IDT pointer
static void _lidt(struct idt_ptr *idtr) {
	__asm__ __volatile__ ("lidt [rdi]" : : "D"(idtr) : "memory" );
}

// Initialize the IDT with default handlers for ISRs
void idt_init() {
	uint8_t i;
	isr_set_gate (0, isr_0, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (1, isr_1, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (2, isr_2, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (3, isr_3, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (4, isr_4, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (5, isr_5, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (6, isr_6, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (7, isr_7, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (8, isr_8, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (9, isr_9, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (10, isr_10, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (11, isr_11, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (12, isr_12, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (13, isr_13, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (14, isr_14, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (16, isr_16, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (17, isr_17, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (18, isr_18, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (19, isr_19, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate (20, isr_20, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	_IDTR.limit = sizeof (_IDT) - 1;
	_IDTR.base = (uint64_t) _IDT;
	_lidt (&_IDTR);
}

// Set an interrupt gate
void isr_set_gate(uint8_t num, void (*gate) (void), uint8_t ist, uint16_t seg, uint8_t flags) {
	uint64_t base = (uint64_t) gate;
	_IDT[num] = _IDT_ZERO;
	_IDT[num].base_low =  (uint16_t) (base & 0xffff);
	_IDT[num].base_mid =  (uint16_t) ((base >> 16) & 0xffff);
	_IDT[num].base_high =  (uint32_t) ((base >> 32) & 0xffffffff);
	_IDT[num].ist = (ist & 0x07);
	_IDT[num].seg = seg;
	_IDT[num].gate_type = flags & 0x0f;
	_IDT[num].storage = (flags >> 4) & 0x01;
	_IDT[num].dpl = (flags >> 5) & 0x03;
	_IDT[num].present = (flags >> 7) & 0x01;
}

// Unset an interrupt gate
void isr_unset_gate(uint8_t num) {
	_IDT[num] = _IDT_ZERO;
}

// String descriptions of known ISR vectors
static const char *_INT_STR[] = {
	"Divide by zero",
	"Debug exception",
	"Non maskable interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND range exceeded",
	"Invalid opcode",
	"Device not available",
	"Double fault",
	"Coprocessor segment overrun",
	"Invalid TSS",
	"Segment not present",
	"Stack segment fault",
	"General protection fault",
	"Page fault",
	NULL,
	"x87 FPU floating-point error",
	"Alignment check",
	"Machine check",
	"SIMD floating point exception",
	"Virtualization exception",
};

// Common C handler for ISRs
void isr_common_c_handler(const struct int_regs *regs) {
	const char *str;
	if (regs->int_num <= 20 && regs->int_num != 15) {
		str = _INT_STR[regs->int_num];
	} else {
		str = "Unknown";
	}
	klog ("Interrupt: (%s)\n"
	      "CS: %#04x, RIP: %#016x, SS: %#04x, RSP: %#016x, Error: %#x\n",
	      str, regs->cs, regs->rip, regs->ss, regs->rsp, regs->err_code);
	while (1) {
		__asm__ __volatile__ ("1: cli; hlt; jmp 1" : : : "memory" );
	}
	__builtin_unreachable ();
}

// (C) 2018 Srimanta Barua

#include <stdint.h>
#include <stddef.h>
#include <tmos/system.h>
#include <tmos/arch/idt.h>
#include <tmos/arch/dev/pic.h>
#include <tmos/klog.h>

// Macro for an exception which does not push error code
#define EXCEPT_NOERROR(i) \
static void __attribute__((naked)) _isr_##i() {      \
	__asm__ __volatile__ ("push 0; push " #i ";" \
			      "jmp _common_landing;" \
			      : : : "memory");       \
}

// Macro for an exception which pushes error code
#define EXCEPT_ERROR(i) \
static void __attribute__((naked)) _isr_##i() {      \
	__asm__ __volatile__ ("push " #i ";"         \
			      "jmp _common_landing;" \
			      : : : "memory");       \
}

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
} __attribute__((packed));

// Structure of an IDT pointer
struct idt_ptr {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed));


// TODO: This should probably be locked..
static struct idt_entry _IDT[256] = { { 0 } };
static struct idt_ptr _IDTR = { 0 };
static const struct idt_entry _IDT_ZERO = { 0 };

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

// Load the IDT pointer
static void _lidt(struct idt_ptr *idtr) {
	__asm__ __volatile__ ("lidt [rdi]" : : "D"(idtr) : "memory" );
}

// Common C handler for ISRs
static void __attribute__((used)) _common_c_handler(const struct int_regs *regs) {
	const char *str;
	if (regs->int_num <= 20 && regs->int_num != 15) {
		str = _INT_STR[regs->int_num];
	} else {
		str = "Unknown";
	}
	klog("Interrupt: (%s)\n"
	      "CS: %#04x, RIP: %#016llx, SS: %#04x, RSP: %#016llx, Error: %#x\n",
	      str, regs->cs, regs->rip, regs->ss, regs->rsp, regs->err_code);
	crash_and_burn();
}

// Common landing point for exception handlers. This pushes registers and
// calls the C handler
static void __attribute__((naked, used)) _common_landing() {
	ISR_PUSH_REGS;
	__asm__ __volatile__ ("mov rdi, rsp;"
			      "call _common_c_handler;"
			      : : :);
	ISR_POP_REGS;
	__asm__ __volatile__ ("add rsp, 16; iretq;" : : : );
}

// Default exception handlers
EXCEPT_NOERROR(0)
EXCEPT_NOERROR(1)
EXCEPT_NOERROR(2)
EXCEPT_NOERROR(3)
EXCEPT_NOERROR(4)
EXCEPT_NOERROR(5)
EXCEPT_NOERROR(6)
EXCEPT_NOERROR(7)
EXCEPT_ERROR(8)
EXCEPT_NOERROR(9)
EXCEPT_ERROR(10)
EXCEPT_ERROR(11)
EXCEPT_ERROR(12)
EXCEPT_ERROR(13)
EXCEPT_NOERROR(16)
EXCEPT_ERROR(17)
EXCEPT_NOERROR(18)
EXCEPT_NOERROR(19)
EXCEPT_NOERROR(20)


// Initialize the IDT with default handlers for ISRs
void idt_init() {
	isr_set_gate(0, _isr_0, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(1, _isr_1, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(2, _isr_2, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(3, _isr_3, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(4, _isr_4, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(5, _isr_5, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(6, _isr_6, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(7, _isr_7, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(8, _isr_8, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(9, _isr_9, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(10, _isr_10, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(11, _isr_11, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(12, _isr_12, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(13, _isr_13, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(16, _isr_16, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(17, _isr_17, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(18, _isr_18, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(19, _isr_19, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	isr_set_gate(20, _isr_20, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	_IDTR.limit = sizeof(_IDT) - 1;
	_IDTR.base = (uint64_t) _IDT;
	_lidt(&_IDTR);
	// Initialize the PIC
	pic_init(IRQ_TIMER, IRQ_RTC);
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

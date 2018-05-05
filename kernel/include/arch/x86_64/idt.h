// (C) 2018 Srimanta Barua
// Interface for central control of interrupts

#pragma once

#include <stdint.h>

// The type flags for ISRs
#define IDT_ATTR_PRESENT 0x80
#define IDT_ATTR_DPL_0   0x00
#define IDT_ATTR_DPL_1   0x20
#define IDT_ATTR_DPL_2   0x40
#define IDT_ATTR_DPL_3   0x60
#define IDT_ATTR_TRAP_32 0x0f
#define IDT_ATTR_INT_32  0x0e
#define IDT_ATTR_TRAP_16 0x07
#define IDT_ATTR_INT_16  0x06
#define IDT_ATTR_TASK_32 0x05

// The registers pushed on the stack by interrupt handlers
struct int_regs {
	// Scratch registers in the System V ABI
	uint64_t r11, r10, r9, r8;
	uint64_t rsi, rdi, rdx, rcx, rax;
	// Other stuff - interrupt number and (possibly dummy) error code
	uint64_t int_num, err_code;
	// Registers pushed by CPU before invoking handler
	uint64_t rip, cs, rflags, rsp, ss;
};

// Initialize the IDT with default handlers for ISRs
void idt_init();

// Set up a custom ISR interrupt gate
void isr_set_gate(uint8_t num, void (*gate) (void), uint8_t ist, uint16_t seg, uint8_t flags);

// Unset custom ISR gate
void isr_unset_gate(uint8_t intnum);

// Disable interrupts
inline void idt_disable_int() {
	__asm__ __volatile__ ("cli" : : : "memory");
}

// Enable interrupts
inline void idt_enable_int() {
	__asm__ __volatile__ ("sti" : : : "memory");
}

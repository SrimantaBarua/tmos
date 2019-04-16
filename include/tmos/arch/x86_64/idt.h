// (C) 2018 Srimanta Barua
// Interface for central control of interrupts

#pragma once

#include <stdint.h>
#include <tmos/system.h>

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

// Known hardware interrupts
#define IRQ_TIMER 0x20
#define IRQ_KBRD  0x21
#define IRQ_RTC   0x28

// Helpful macros for writing interrupt handlers
#define ISR_PUSH_REGS \
	__asm__ __volatile__ ("push rax; push rcx; push rdx; push rdi;" \
			      "push rsi; push r8; push r9; push r10;" \
			      "push r11; " : : : "memory" )

#define ISR_POP_REGS \
	__asm__ __volatile__ ("pop r11; pop r10; pop r9; pop r8; pop rsi;" \
			      "pop rdi; pop rdx; pop rcx; pop rax;" \
			      : : : "memory" )

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
static inline void idt_disable_int() {
	__asm__ __volatile__ ("cli" : : : "memory");
}

// Enable interrupts
static inline void idt_enable_int() {
	__asm__ __volatile__ ("sti" : : : "memory");
}

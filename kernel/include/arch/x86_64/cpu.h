// (C) 2018 Srimanta Barua
//
// Structures and instructions specific to the x86_64 CPU

#pragma once

#include <stdint.h>
#include <system.h>
#include <arch/x86_64/ctrlreg.h>
#include <arch/x86_64/msr.h>

// The CPU RFLAGS register
union rflags {
	uint64_t raw;
	struct {
		uint64_t CF       : 1;  // Carry flag
		uint64_t _rsvd1_0 : 1;  // Always 1
		uint64_t PF       : 1;  // Parity flag
		uint64_t _rsvd0_0 : 1;  // Must be zero
		uint64_t AF       : 1;  // Auxiliary flag
		uint64_t _rsvd0_1 : 1;  // Must be 0
		uint64_t ZF       : 1;  // Zero flag
		uint64_t SF       : 1;  // Sign flag
		uint64_t TF       : 1;  // Trap flag
		uint64_t IF       : 1;  // Interrupt flag
		uint64_t DF       : 1;  // Direction flag
		uint64_t OF       : 1;  // Overflow flag
		uint64_t IOPL     : 2;  // IO privilege level
		uint64_t NT       : 1;  // Nested task
		uint64_t _rsvd0_2 : 1;  // 1 on 8086 and 186, 0 on later models
		uint64_t RF       : 1;  // Resume flag
		uint64_t VM       : 1;  // Virtual 8086 mode
		uint64_t AC       : 1;  // Alignment check
		uint64_t VIF      : 1;  // Virtual interrupt flag
		uint64_t VIP      : 1;  // Virtual interrupt pending
		uint64_t ID       : 1;  // Able to use CPUID
		uint64_t _rsvd0_3 : 42; // Always 0
	} PACKED f;
};

// Read the RFLAGS register
static inline union rflags cpu_read_rflags() {
	union rflags ret;
	__asm__ __volatile__ ("pushfq; pop rax;" : "=a"(ret.raw) : : );
	return ret;
}

// Write the RFLAGS register
static inline void cpu_write_rflags(union rflags flags) {
	__asm__ __volatile__ ("push rdi; popfq" : : "D"(flags.raw) : "memory" );
}

// Enable exec protection, so that we can't execute code from pages marked NO_EXEC
static inline void set_nx() {
	wrmsr(MSR_EFER, rdmsr(MSR_EFER) | MSR_EFER_NXE);
}

// Enable write protection, so that we can't write to pages unless marked WRITABLE
static inline void set_write_protect() {
	write_cr0(read_cr0() | CR0_WRITE_PROTECT);
}

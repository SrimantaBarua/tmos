// (C) 2018 Srimanta Barua
// Reading and writing Model Specific Registers (MSRs)

#pragma once

#include <system.h>

// MSR numbers
#define MSR_EFER 0xC0000080


// Bits in the EFER MSR
#define MSR_EFER_SYSCALL  (1 << 0)
#define MSR_EFER_LONGMODE (1 << 8)
#define MSR_EFER_LMA      (1 << 10)
#define MSR_EFER_NXE      (1 << 11)
#define MSR_EFER_SVME     (1 << 12)
#define MSR_EFER_LMSLE    (1 << 13)
#define MSR_EFER_FFXSR    (1 << 14)
#define MSR_EFER_TCE      (1 << 15)

// Read an MSR
FORCEINLINE uint64_t rdmsr(uint32_t num) {
	uint32_t eax, edx;
	__asm__ __volatile__ ("rdmsr\n" : "=a"(eax), "=d"(edx) : "c"(num) : );
	return ((uint64_t) edx << 32) | eax;
}

// Write an MSR
FORCEINLINE void wrmsr(uint32_t num, uint64_t val) {
	uint32_t eax, edx;
	edx = (val >> 32) & 0xffffffff;
	eax = val & 0xffffffff;
	__asm__ __volatile__ ("wrmsr\n" : : "a"(eax), "d"(edx), "c"(num) : "memory");
}

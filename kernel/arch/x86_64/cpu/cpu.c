// (C) 2018 Srimanta Barua

#include <arch/x86_64/cpu.h>
#include <arch/x86_64/tss.h>

// Buffer for holding TSS data
char __tss_buf[__MAX_NUM_TSS__ * __BYTES_PER_TSS__] __attribute__ ((aligned(__BYTES_PER_TSS__)));

// Stop forever
void __attribute__((noreturn)) crash_and_burn() {
	while (1) {
		__asm__ __volatile__ ("cli; hlt;" : : : "memory");
	}
	__builtin_unreachable();
}

// (C) 2018 Srimanta Barua

#include <tmos/arch/cpu.h>
#include <tmos/arch/tss.h>

// Buffer for holding TSS data
char __tss_buf[__TMOS_CFG_MAX_NUM_TSS__ * __TMOS_CFG_BYTES_PER_TSS__] __attribute__ ((aligned(__TMOS_CFG_BYTES_PER_TSS__)));

// Stop forever
void __attribute__((noreturn)) crash_and_burn() {
	while (1) {
		__asm__ __volatile__ ("cli; hlt;" : : : "memory");
	}
	__builtin_unreachable();
}

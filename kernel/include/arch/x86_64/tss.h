// (C) 2018 Srimanta Barua
//
// Task state segment. It is almost pointless in long mode, because task switching is done only
// in software. But it is still needed. Meh.
// Only important parts are priviledged stacks, IST, and IO permissions bitmap
//
// TSSes should not cross a page bounary. Plus we should probably leave some space for IO
// permissions bitmaps (even though we're not using them now). So we'll leave 128 bytes for TSSes,
// thus allocating 32 per page.

#pragma once

#include <system.h>
#include <stdint.h>
#include <klog.h>

// TSS structure
struct tss {
	uint32_t __rsvd0; // Must be 0
	uint64_t rsp0;    // Priviledged stacks
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t __rsvd1; // Must be 0
	uint64_t ist0;    // Interrupt stack table
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t __rsvd2;
	uint16_t __rsvd3;
	uint16_t ioperm_off; // 16-bit offset to IO permissions bitmap from base of TSS
} PACKET;

#ifndef __CFG_SMP__
#define __MAX_NUM_TSS__ 1
#else
#define __MAX_NUM_TSS__ __CFG_MAX_NUM_CPUS__
#endif

#define __BYTES_PER_TSS__ 128

// Buffer for TSSes
extern char __tss_buf[];

// Get pointer to nth TSS
struct tss* tss_get_n(uint32_t n) {
	ASSERT(n < __MAX_NUM_TSS__);
	return (struct tss*) (__tss_buf + (n * __BYTES_PER_TSS__));
}

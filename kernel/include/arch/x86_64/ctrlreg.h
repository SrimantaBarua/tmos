// (C) 2018 Srimanta Barua
// Reading and writing control registers

#pragma once

#include <system.h>

// Bits in CR0
#define CR0_PROTECTED_MODE    (1 << 0)
#define CR0_MONITOR_COPROC    (1 << 1)
#define CR0_EMULATION         (1 << 2)
#define CR0_TASK_SWITCHED     (1 << 3)
#define CR0_EXTENSION_TYPE    (1 << 4)
#define CR0_NUMERIC_ERROR     (1 << 5)
#define CR0_WRITE_PROTECT     (1 << 16)
#define CR0_NOT_WRITE_THROUGH (1 << 29)
#define CR0_CACHE_DISABLE     (1 << 30)
#define CR0_PAGING            (1 << 31)

// CR1 is reserved

// CR2 is page fault linear address

// CR3 is PML4 address. Bits in CR3
#define CR3_PML4_PADDR_MASK          0x000ffffffffff000
#define CR3_PAGE_LEVEL_CACHE_DISABLE (1 << 4)
#define CR3_PAGE_LEVEL_WRITE_THROUGH (1 << 3)

// Bits in CR4
#define CR4_V8086_MODE_EXT      (1 << 0)
#define CR4_PMODE_VIRT_INT      (1 << 1)
#define CR4_TIMESTAMP_DISABLE   (1 << 2)
#define CR4_DEBUG_EXT           (1 << 3)
#define CR4_PAGE_SIZE_EXT       (1 << 4)
#define CR4_PADDR_EXT           (1 << 5)
#define CR4_MACHINE_CHECK       (1 << 6)
#define CR4_PAGE_GLOB_ENABLE    (1 << 7)
#define CR4_PERF_CTR_ENABLE     (1 << 8)
#define CR4_OSFXSR              (1 << 9)
#define CR4_OSXMMEXCPT          (1 << 10)
#define CR4_USERMODE_INSTR_PREV (1 << 11)
#define CR4_VMX_ENABLE          (1 << 13)
#define CR4_SMX_ENABLE          (1 << 14)
#define CR4_FSGSBASE            (1 << 16)
#define CR4_PCID_ENABLE         (1 << 17)
#define CR4_OSXSAVE             (1 << 18)
#define CR4_SMEP_ENABLE         (1 << 20)
#define CR4_SMAP_ENABLE         (1 << 21)
#define CR4_PROT_KEY_ENABLE     (1 << 22)

// Bitmask in CR8
#define CR8_TASK_PRIORITY_LEVEL_MASK 0x0F

// Read CR0
static inline uint64_t read_cr0() {
	uint64_t ret;
	__asm__ __volatile__ ("mov rax, cr0\n" : "=a"(ret) : : );
	return ret;
}

// Write CR0
static inline uint64_t write_cr0(uint64_t val) {
	__asm__ __volatile__ ("mov cr0, rdi\n" : : "D"(val) : "memory" );
}

// Read CR2
static inline uint64_t read_cr2() {
	uint64_t ret;
	__asm__ __volatile__ ("mov rax, cr2\n" : "=a"(ret) : : );
	return ret;
}

// Write CR2
static inline uint64_t write_cr2(uint64_t val) {
	__asm__ __volatile__ ("mov cr2, rdi\n" : : "D"(val) : "memory" );
}

// Read the CR3 control register (PML4 address)
static inline uint64_t read_cr3() {
	uint64_t ret;
	__asm__ __volatile__ ("mov rax, cr3\n" : "=a"(ret) : : );
	return ret;
}

// Write the CR3 control register (PML4 address)
static inline uint64_t write_cr3(uint64_t val) {
	__asm__ __volatile__ ("mov cr3, rdi\n" : : "D"(val) : "memory" );
}

// Read CR4
static inline uint64_t read_cr4() {
	uint64_t ret;
	__asm__ __volatile__ ("mov rax, cr4\n" : "=a"(ret) : : );
	return ret;
}

// Write CR4
static inline uint64_t write_cr4(uint64_t val) {
	__asm__ __volatile__ ("mov cr4, rdi\n" : : "D"(val) : "memory" );
}

// Read CR8
static inline uint64_t read_cr8() {
	uint64_t ret;
	__asm__ __volatile__ ("mov rax, cr8\n" : "=a"(ret) : : );
	return ret;
}

// Write CR8
static inline uint64_t write_cr8(uint64_t val) {
	__asm__ __volatile__ ("mov cr8, rdi\n" : : "D"(val) : "memory" );
}

// (C) 2018 Srimanta Barua

#pragma once

#include <arch/x86_64/cpu.h>
#include <arch/x86_64/idt.h>

// Paging
#define PAGE_SIZE       4096
#define PAGE_SIZE_SHIFT 12

// Word sizes
#define WORD_SIZE       64
#define WORD_SIZE_SHIFT 6

typedef uint64_t vaddr_t ;
typedef uint64_t paddr_t ;

// Symbols from linker.ld
extern int __kernel_vbase__;
extern int __kernel_phys_start__;
extern int __kernel_phys_end__;
extern int __kernel_virt_start__;
extern int __kernel_virt_end__;
extern int __text_start__;
extern int __text_end__;
extern int __data_start__;
extern int __data_end__;
extern int __rodata_start__;
extern int __rodata_end__;
extern int __bss_start__;
extern int __bss_end__;

// Get linker.ld symbol addresses
#define __SYM_ADDR__(x) ((vaddr_t) &(x))
#define KRNL_VBASE        __SYM_ADDR__(__kernel_vbase__);
#define KRNL_PHYS_START   __SYM_ADDR__(__kernel_phys_start__);
#define KRNL_PHYS_END     __SYM_ADDR__(__kernel_phys_end__);
#define KRNL_VIRT_START   __SYM_ADDR__(__kernel_virt_start__);
#define KRNL_VIRT_END     __SYM_ADDR__(__kernel_virt_end__);
#define KRNL_TEXT_START   __SYM_ADDR__(__kernel_text_start__);
#define KRNL_TEXT_END     __SYM_ADDR__(__kernel_text_end__);
#define KRNL_DATA_START   __SYM_ADDR__(__kernel_data_start__);
#define KRNL_DATA_END     __SYM_ADDR__(__kernel_data_end__);
#define KRNL_RODATA_START __SYM_ADDR__(__kernel_rodata_start__);
#define KRNL_RODATA_END   __SYM_ADDR__(__kernel_rodata_end__);
#define KRNL_BSS_START    __SYM_ADDR__(__kernel_bss_start__);
#define KRNL_BSS_END      __SYM_ADDR__(__kernel_bss_end__);

// Check if interrupts are enabled
#define sys_int_enabled() (cpu_read_rflags ().f.IF == 1)

// Disable interrupts
#define sys_disable_int() idt_disable_int ()

// Enable interrupts
#define sys_enable_int() idt_enable_int ()

// Die
#define crash_and_burn() \
	__asm__ __volatile__ ("1: cli; hlt; jmp 1" : : : "memory");


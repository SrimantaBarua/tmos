// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>

// GDT descriptor flags (only the ones which are relevant in long mode)
#define GDT_DESC_RW         ((uint64_t) 1 << 41)
#define GDT_DESC_CONFORM    ((uint64_t) 1 << 42)
#define GDT_DESC_EXEC       ((uint64_t) 1 << 43)
#define GDT_DESC_USER       ((uint64_t) 1 << 44)
#define GDT_DESC_PRESENT    ((uint64_t) 1 << 47)
#define GDT_DESC_LONG_MODE  ((uint64_t) 1 << 53)
#define GDT_DESC_DPL_SHIFT  45

// A kernel code segment
#define KRNL_CODE_SEGMENT \
	(GDT_DESC_LONG_MODE | GDT_DESC_PRESENT | GDT_DESC_USER | GDT_DESC_EXEC | GDT_DESC_RW)
#define KRNL_DATA_SEGMENT \
	(GDT_DESC_LONG_MODE | GDT_DESC_PRESENT | GDT_DESC_USER | GDT_DESC_CONFORM | GDT_DESC_RW)
#define USER_CODE_SEGMENT \
	(GDT_DESC_LONG_MODE | ((uint64_t) 3 << GDT_DESC_DPL_SHIFT) | GDT_DESC_PRESENT | GDT_DESC_USER | GDT_DESC_EXEC | GDT_DESC_RW)
#define USER_DATA_SEGMENT \
	(GDT_DESC_LONG_MODE | ((uint64_t) 3 << GDT_DESC_DPL_SHIFT) | GDT_DESC_PRESENT | GDT_DESC_USER | GDT_DESC_CONFORM | GDT_DESC_RW)

// Initialize the GDT
void gdt_init();

// Add a user segment
uint16_t gdt_add_seg(uint64_t seg);

// Add a TSS
uint16_t gdt_add_tss(const void *tss);

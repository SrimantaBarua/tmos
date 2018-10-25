// (C) 2018 Srimanta Barua
//
// Arch-specific interface for memory management. Virtual memory and the like

#pragma once

#include <shuos/system.h>
#include <shuos/memory.h>

// Flags for a page table entry
#define PTE_FLG_PRESENT        ((uint64_t) (1 << 0))
#define PTE_FLG_WRITABLE       ((uint64_t) (1 << 1))
#define PTE_FLG_USER_ACCESS    ((uint64_t) (1 << 2))
#define PTE_FLG_WRITE_THROUGH  ((uint64_t) (1 << 3))
#define PTE_FLG_NO_CACHE       ((uint64_t) (1 << 4))
#define PTE_FLG_ACCESSED       ((uint64_t) (1 << 5))
#define PTE_FLG_DIRTY          ((uint64_t) (1 << 6))
#define PTE_FLG_HUGE_PAGE      ((uint64_t) (1 << 7))
#define PTE_FLG_GLOBAL         ((uint64_t) (1 << 8))
#define PTE_FLG_TO_ALLOC       ((uint64_t) (1 << 9))
#define PTE_PADDR_MASK         ((uint64_t) 0x000ffffffffff000)
#define PTE_FLG_NO_EXEC        ((uint64_t) 0x8000000000000000)

// Initialize virtual memory manager
void vmm_init(struct pmmgr *pmmgr, void (*remap_cb) (void));

// Allocate and map n virtual memory pages with the given flags
void vmm_map(vaddr_t addr, uint64_t n, uint64_t flags);

// Free n virtual memory pages
void vmm_free(vaddr_t vaddr, uint64_t n);

// Map a virtual page to a given physical frame with the given flags
void vmm_map_to(vaddr_t vaddr, paddr_t paddr, uint64_t n, uint64_t flags);

// Translate a virtual address to a physical address
paddr_t vmm_translate(vaddr_t vaddr);

// Unmap n virtual memory pages
void vmm_unmap(vaddr_t vaddr, uint64_t n);

// Print the page table structure
void vmm_print_ptable();

// Switch address space to PML4 at given paddr, and return paddr of current PML4
paddr_t vmm_switch_addr_space(paddr_t new_pml4_addr);

// Invalidate a page table entry
void invlpg(vaddr_t addr);

// Invalidate the whole TLP
void tlb_flush_all();

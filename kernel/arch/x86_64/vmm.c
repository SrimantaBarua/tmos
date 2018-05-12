// (C) 2018 Srimanta Barua

#include <system.h>
#include <memory.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <klog.h>
#include <arch/x86_64/memory.h>

// A page table
struct ptable {
	uint64_t e[512];
};

// Virtual addr for PML4
#define PML4_VADDR 0xFFFFFF7FBFDFE000

// Check and set unused
#define PTE_UNUSED(e) ((e) == 0)

// Check flags
#define PTE_PRESENT(e)     (((e) & PTE_FLG_PRESENT) != 0)
#define PTE_WRITABLE(e)    (((e) & PTE_FLG_WRITABLE) != 0)
#define PTE_USER_ACCESS(e) (((e) & PTE_FLG_USER_ACCESS) != 0)
#define PTE_WRITETHRU(e)   (((e) & PTE_FLG_WRITE_THROUGH) != 0)
#define PTE_NOCACHE(e)     (((e) & PTE_FLG_NO_CACHE) != 0)
#define PTE_ACCESSED(e)    (((e) & PTE_FLG_ACCESSED) != 0)
#define PTE_DIRTY(e)       (((e) & PTE_FLG_DIRTY) != 0)
#define PTE_HUGE(e)        (((e) & PTE_FLG_HUGE_PAGE) != 0)
#define PTE_GLOBAL(e)      (((e) & PTE_FLG_GLOBAL) != 0)
#define PTE_TO_ALLOC(e)    (((e) & PTE_FLG_TO_ALLOC) != 0)
#define PTE_NOEXEC(e)      (((e) & PTE_FLG_NO_EXEC) != 0)

// Set and unset flag
#define PTE_SET_FLG(e,flag)   do { e |= flag; } while (0)
#define PTE_UNSET_FLG(e,flag) do { e &= ~(flag); } while (0)

// Set and get paddr that an entry points to
#define PTE_PADDR(e) ((e) & PTE_PADDR_MASK)
#define PTE_SET_PADDR(e,addr) do { \
	e &= ~PTE_PADDR_MASK;      \
	e |= addr;                 \
} while (0);
#define PTE_SET(e,addr,flags) do { \
	e = addr | flags;          \
} while (0);

// Get table index for given address
#define PML4_IDX(addr) (((vaddr_t) (addr) >> 39) & 0x1ff)
#define PDP_IDX(addr) (((vaddr_t) (addr) >> 30) & 0x1ff)
#define PD_IDX(addr) (((vaddr_t) (addr) >> 21) & 0x1ff)
#define PT_IDX(addr) (((vaddr_t) (addr) >> 12) & 0x1ff)

// Check if a virtual address is valid (everything beyond 48 bits sign extended) and page aligned
#define VADDR_IS_VALID(addr) \
	(((addr) & 0xfff8000000000fff) == 0 || ((addr) & 0xfff8000000000fff) == 0xfff8000000000000)

// Get address of child table at given index, given address of parent
#define PT_CHILD(parent, idx) \
	((struct ptable*) ((((vaddr_t) (parent) << 9) | 0xffff000000000000) | ((idx) << 12)))

static struct pmmgr *_PMMGR = NULL;

// Returns the child table. If not present, or huge, return NULL
static struct ptable* _pt_child(const struct ptable *tab, uint64_t idx) {
	if (PTE_PRESENT (tab->e[idx]) &&  !PTE_HUGE (tab->e[idx])) {
		return (struct ptable*) PT_CHILD (tab, idx);
	}
	return NULL;
}

// Check if a table exists. If yes, return. Else create and return
static struct ptable* _pt_create(struct ptable *tab, uint64_t idx) {
	paddr_t paddr;
	struct ptable *ptr;
	if (PTE_PRESENT (tab->e[idx])) {
		if (!PTE_HUGE (tab->e[idx])) {
			return PT_CHILD (tab, idx);
		}
		return NULL;
	}
	// Not present, create
	ASSERT ((paddr = _PMMGR->alloc ()) != PADDR_INVALID);
	tab->e[idx] = 0;
	PTE_SET (tab->e[idx], paddr, PTE_FLG_PRESENT | PTE_FLG_WRITABLE);
	// Zero out the entry and return
	ptr = (struct ptable*) PT_CHILD (tab, idx);
	memset (ptr, 0, sizeof (struct ptable));
	return ptr;
}

// Internal free function to reduce code
static void _do_free(vaddr_t vaddr, uint64_t n, bool do_free) {
	struct ptable *pml4, *pdp, *pd, *pt;
	uint64_t idx, i, j;
	bool flag;
	paddr_t paddr;
	// Check we're initialized
	ASSERT (_PMMGR);
	// Check if address is valid
	ASSERT (vaddr);
	ASSERT (VADDR_IS_VALID (vaddr));
	pml4 = (struct ptable*) PML4_VADDR;
	for (i = 0; i < n; i++) {
		ASSERT (pdp = _pt_child (pml4, PML4_IDX (vaddr)));
		ASSERT (pd = _pt_child (pdp, PDP_IDX (vaddr)));
		ASSERT (pt = _pt_child (pd, PD_IDX (vaddr)));
		idx = PT_IDX (vaddr);
		// The entry shouldn't be unused because that's what we're going to do now
		ASSERT (!PTE_UNUSED (pt->e[idx]));
		if (do_free) {
			paddr = PTE_PADDR (pt->e[idx]);
			_PMMGR->free (paddr);
		}
		pt->e[idx] = 0;
		invlpg (vaddr);
		// If tables are empty, free tables
		flag = 1;
		for (j = 0; j < 512; j++) {
			if (!PTE_UNUSED (pt->e[j])) {
				flag = 0;
				break;
			}
		}
		if (!flag) {
			vaddr += PAGE_SIZE;
			continue;
		}
		idx = PD_IDX (vaddr);
		paddr = PTE_PADDR (pd->e[idx]);
		_PMMGR->free (paddr);
		pd->e[idx] = 0;
		flag = 1;
		for (j = 0; j < 512; j++) {
			if (!PTE_UNUSED (pd->e[j])) {
				flag = 0;
				break;
			}
		}
		if (!flag) {
			vaddr += PAGE_SIZE;
			continue;
		}
		idx = PDP_IDX (vaddr);
		paddr = PTE_PADDR (pdp->e[idx]);
		_PMMGR->free (paddr);
		pdp->e[idx] = 0;
		for (j = 0; j < 512; j++) {
			if (!PTE_UNUSED (pdp->e[j])) {
				flag = 0;
				break;
			}
		}
		if (!flag) {
			vaddr += PAGE_SIZE;
			continue;
		}
		idx = PML4_IDX (vaddr);
		paddr = PTE_PADDR (pml4->e[idx]);
		_PMMGR->free (paddr);
		pml4->e[idx] = 0;
		vaddr += PAGE_SIZE;
	}
}

// Initialize the memory management subsystem with the given underlying physical memory manager
void mem_init(struct pmmgr *pmmgr) {
	ASSERT (!_PMMGR);
	ASSERT (pmmgr);
	_PMMGR = pmmgr;

	// TODO: Switch to new page table and map kernel correctly
}

// Allocate and map n virtual memory pages with the given flags, at the given addresss
void vmm_map(vaddr_t vaddr, uint64_t n, uint64_t flags) {
	struct ptable *pml4, *pdp, *pd, *pt;
	uint64_t idx, i;
	paddr_t paddr;
	// Check we're initialized
	ASSERT (_PMMGR);
	// Check if addresses are valid
	ASSERT (vaddr);
	ASSERT (VADDR_IS_VALID (vaddr));
	// If tables are not present, create then. If couldn't create, panic
	pml4 = (struct ptable*) PML4_VADDR;
	for (i = 0; i < n; i++) {
		ASSERT (pdp = _pt_create (pml4, PML4_IDX (vaddr)));
		ASSERT (pd = _pt_create (pdp, PDP_IDX (vaddr)));
		ASSERT (pt = _pt_create (pd, PD_IDX (vaddr)));
		idx = PT_IDX (vaddr);
		// Allocate paddr
		paddr = _PMMGR->alloc ();
		// Check that PT entry is unused
		ASSERT (PTE_UNUSED (pt->e[idx]));
		PTE_SET (pt->e[idx], paddr, flags | PTE_FLG_PRESENT);
		// Go to next page
		vaddr += PAGE_SIZE;
	}
}

// Free n virtual memory pages
void vmm_free(vaddr_t vaddr, uint64_t n) {
	_do_free (vaddr, n, true);
}

// Map n virtual page to a given physical frame with the given flags
void vmm_map_to(vaddr_t vaddr, paddr_t paddr, uint64_t n, uint64_t flags) {
	struct ptable *pml4, *pdp, *pd, *pt;
	uint64_t idx, i;
	// Check we're initialized
	ASSERT (_PMMGR);
	// Check if addresses are valid
	ASSERT (vaddr);
	ASSERT (VADDR_IS_VALID (vaddr));
	ASSERT (!(paddr & ~PADDR_ALGN_MASK));
	// If tables are not present, create then. If couldn't create, panic
	pml4 = (struct ptable*) PML4_VADDR;
	for (i = 0; i < n; i++) {
		ASSERT (pdp = _pt_create (pml4, PML4_IDX (vaddr)));
		ASSERT (pd = _pt_create (pdp, PDP_IDX (vaddr)));
		ASSERT (pt = _pt_create (pd, PD_IDX (vaddr)));
		idx = PT_IDX (vaddr);
		// Check that PT entry is unused
		ASSERT (PTE_UNUSED (pt->e[idx]));
		PTE_SET (pt->e[idx], paddr, flags | PTE_FLG_PRESENT);
		// Go to next page
		vaddr += PAGE_SIZE;
		paddr += PAGE_SIZE;
	}
}

// Unmap n virtual memory pages
void vmm_unmap(vaddr_t vaddr, uint64_t n) {
	_do_free (vaddr, n, false);
}

// Print the page table structure
void vmm_print_ptable() {
	uint64_t i, j, k, l;
	vaddr_t vaddr;
	struct ptable *pml4, *pdp, *pd, *pt;
	pml4 = (struct ptable*) PML4_VADDR;
	klog ("\nPML4: %#llx\n", pml4);
	for (i = 0; i < 512; i++) {
		if (PTE_UNUSED (pml4->e[i])) {
			continue;
		}
		klog ("pml4[%u] -> %#llx\n", i, PTE_PADDR (pml4->e[i]));
		ASSERT (pdp = _pt_child (pml4, i));
		for (j = 0; j < 512; j++) {
			if (PTE_UNUSED (pdp->e[j])) {
				continue;
			}
			klog ("  pdp[%u] -> %#llx\n", j, PTE_PADDR (pdp->e[j]));
			if (PTE_HUGE (pdp->e[j])) {
				continue;
			}
			ASSERT (pd = _pt_child (pdp, j));
			for (k = 0; k < 512; k++) {
				if (PTE_UNUSED (pd->e[k])) {
					continue;
				}
				klog ("    pd[%u] -> %#llx\n", k, PTE_PADDR (pd->e[k]));
				if (PTE_HUGE (pd->e[k])) {
					continue;
				}
				ASSERT (pt = _pt_child (pd, k));
				for (l = 0; l < 512; l++) {
					if (PTE_UNUSED (pt->e[l])) {
						continue;
					}
					vaddr = (l << 12) + (k << 21) + (j << 30) + (i << 39);
					if (i > 255) {
						vaddr |= 0xffff000000000000;
					}
					klog ("      pt[%u] -> %#llx  --  (%#llx)\n",
					      l, PTE_PADDR (pt->e[l]), vaddr);
				}
			}
		}
	}
}

// Invalidate a page table entry
void invlpg(vaddr_t addr) {
	__asm__ __volatile__ ("invlpg [%0]\n" : : "r"(addr) : "memory");
}

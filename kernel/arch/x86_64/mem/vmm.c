// (C) 2018 Srimanta Barua

#include <system.h>
#include <memory.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <klog.h>
#include <arch/x86_64/memory.h>
#include <arch/x86_64/cpu.h>

// A page table
struct ptable {
	uint64_t e[512];
};

// Virtual addr for PML4
#define PML4_VADDR 0xffffff7fbfdfe000
#define TEMP_VADDR 0xffffff8000000000

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

// Get all flags
#define PTE_FLAGS(e) ((e) & ~PTE_PADDR_MASK)

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
	(((addr) & 0xfff8000000000000) == 0 || ((addr) & 0xfff8000000000000) == 0xfff8000000000000)

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
	memset (ptr, 0, PAGE_SIZE);
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
	ASSERT (!(vaddr & 0xfff));
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

// Set up a PML4. Allocate a frame, zero it out, map to itself at index 510. Return paddr
static paddr_t _setup_new_pml4() {
	struct ptable *ptr;
	paddr_t paddr;
	// Allocate a frame for the PML4
	ASSERT ((paddr = _PMMGR->alloc ()) != PADDR_INVALID);
	// Map it to a temporary address
	vmm_map_to (TEMP_VADDR, paddr, 1, PTE_FLG_PRESENT | PTE_FLG_WRITABLE);
	ptr = (struct ptable *) TEMP_VADDR;
	// Zero it out
	memset (ptr, 0, PAGE_SIZE);
	// Map 510th entry to itself
	ptr->e[510] = paddr | PTE_FLG_PRESENT | PTE_FLG_WRITABLE;
	// Unmap the temporary page and return
	vmm_unmap (TEMP_VADDR, 1);
	return paddr;
}

// Call a function with a new PML4, and then restore the current PML4
static void _do_with_new_pml4(paddr_t pml4_frame, void (*fn) (void)) {
	struct ptable *ptr;
	paddr_t backup;
	// Backup up current PML4 paddr
	backup = read_cr3 () & PTE_PADDR_MASK;
	// Map the temporary page to backup frame
	vmm_map_to (TEMP_VADDR, backup, 1, PTE_FLG_PRESENT | PTE_FLG_WRITABLE);
	// Overwrite the recursive mapping and map it to temporary pml4
	ptr = (struct ptable *) TEMP_VADDR;
	ptr->e[510] = pml4_frame | PTE_FLG_PRESENT | PTE_FLG_WRITABLE;
	// Invalidate whole TLB
	tlb_flush_all ();
	// Execute the function in the new context
	fn ();
	// Restore recursive mapping
	ptr->e[510] = backup | PTE_FLG_PRESENT | PTE_FLG_WRITABLE;
	tlb_flush_all ();
	// Unmap the remporary page mapiping
	vmm_unmap (TEMP_VADDR, 1);
}

// Initialize the memory management subsystem with the given underlying physical memory manager
// Set up a new page table, with the callback provided (Panic if not provided)
// Switch to the new address space
void mem_init(struct pmmgr *pmmgr, void (*remap_cb) (void)) {
	paddr_t pml4_paddr;

	ASSERT (!_PMMGR);
	ASSERT (pmmgr);
	ASSERT (remap_cb);
	_PMMGR = pmmgr;

	// Allocate new PML4
	pml4_paddr = _setup_new_pml4 ();

	// Remap the kernel with the new PML4
	_do_with_new_pml4 (pml4_paddr, remap_cb);

	// Remap the physical memory manager if it requires it
	if (_PMMGR->remap_cb) {
		_do_with_new_pml4 (pml4_paddr, _PMMGR->remap_cb);
	}

	// Enable noexec and write protection (TODO)
	set_nx ();
	set_write_protect ();

	// Switch to new address space
	vmm_switch_addr_space (pml4_paddr);
}

// Switch address space to PML4 at given paddr, and return paddr of current PML4
paddr_t vmm_switch_addr_space(paddr_t new_pml4_addr) {
	paddr_t ret = read_cr3 () & PTE_PADDR_MASK;
	write_cr3 (new_pml4_addr);
	return ret;
}

// Allocate and map n virtual memory pages with the given flags, at the given addresss
void vmm_map(vaddr_t vaddr, uint64_t n, uint64_t flags) {
	struct ptable *pml4, *pdp, *pd, *pt;
	uint64_t idx, i;
	// Check we're initialized
	ASSERT (_PMMGR);
	// Check if addresses are valid
	ASSERT (vaddr);
	ASSERT (VADDR_IS_VALID (vaddr));
	ASSERT (!(vaddr & 0xfff));
	// If tables are not present, create then. If couldn't create, panic
	pml4 = (struct ptable*) PML4_VADDR;
	for (i = 0; i < n; i++) {
		ASSERT (pdp = _pt_create (pml4, PML4_IDX (vaddr)));
		ASSERT (pd = _pt_create (pdp, PDP_IDX (vaddr)));
		ASSERT (pt = _pt_create (pd, PD_IDX (vaddr)));
		idx = PT_IDX (vaddr);
		// Check that PT entry is unused
		ASSERT (PTE_UNUSED (pt->e[idx]));
		// Mark as to be allocated
		PTE_SET (pt->e[idx], 0, (flags | PTE_FLG_TO_ALLOC) & ~PTE_FLG_PRESENT);
		// Go to next page
		vaddr += PAGE_SIZE;
	}
}

// Free n virtual memory pages
void vmm_free(vaddr_t vaddr, uint64_t n) {
	_do_free (vaddr, n, true);
}

// Translate a virtual address to a physical, returning PADDR_INVALID if unmapped
paddr_t vmm_translate(vaddr_t vaddr) {
	struct ptable *pml4, *pdp, *pd, *pt;
	uint64_t idx;
	vaddr_t off, pd_off, pdp_off;
	// Check if vaddr is valid
	ASSERT (VADDR_IS_VALID (vaddr));
	pml4 = (struct ptable*) PML4_VADDR;
	// Get offset into ptable
	off = vaddr & 0xfff;
	pd_off = vaddr & ((0x1000 << 9) - 1);
	pdp_off = vaddr & ((0x1000 << (9 + 9)) - 1);
	// Get pdp
	if (!(pdp = _pt_child (pml4, PML4_IDX (vaddr)))) {
		return PADDR_INVALID;
	}
	idx = PDP_IDX (vaddr);
	if (!PTE_PRESENT (pdp->e[idx])) {
		return PADDR_INVALID;
	}
	if (PTE_HUGE (pdp->e[idx])) {
		return PTE_PADDR (pdp->e[idx]) + pdp_off;
	}
	if (!(pd = _pt_child (pdp, idx))) {
		return PADDR_INVALID;
	}
	idx = PD_IDX (vaddr);
	if (!PTE_PRESENT (pd->e[idx]))  {
		return PADDR_INVALID;
	}
	if (PTE_HUGE (pd->e[idx])) {
		return PTE_PADDR (pd->e[idx]) + pd_off;
	}
	if (!(pt = _pt_child (pd, idx))) {
		return PADDR_INVALID;
	}
	idx = PT_IDX (vaddr);
	if (PTE_PRESENT (pt->e[idx])) {
		return PTE_PADDR (pt->e[idx]) + off;
	}
	return PADDR_INVALID;
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
	ASSERT (!(vaddr & 0xfff));
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
		klog ("pml4[%u] -> %#llx\n", i, (pml4->e[i]));
		ASSERT (pdp = _pt_child (pml4, i));
		for (j = 0; j < 512; j++) {
			if (PTE_UNUSED (pdp->e[j])) {
				continue;
			}
			klog ("  pdp[%u] -> %#llx\n", j, (pdp->e[j]));
			if (PTE_HUGE (pdp->e[j])) {
				continue;
			}
			ASSERT (pd = _pt_child (pdp, j));
			for (k = 0; k < 512; k++) {
				if (PTE_UNUSED (pd->e[k])) {
					continue;
				}
				klog ("    pd[%u] -> %#llx\n", k, (pd->e[k]));
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
					      l, (pt->e[l]), vaddr);
				}
			}
		}
	}
}

// Invalidate a page table entry
void invlpg(vaddr_t addr) {
	__asm__ __volatile__ ("invlpg [%0]\n" : : "r"(addr) : "memory");
}

// Invalidate the whole TLP
void tlb_flush_all() {
	__asm__ __volatile__ ("mov rax, cr3; mov cr3, rax\n" : : : "rax", "memory");
}


// PAGE FAULT HANDLER

// Error code bits
#define ERR_CODE_PRESENT 1
#define ERR_CODE_WRITE   2
#define ERR_CODE_USER    4
#define ERR_CODE_RSVD    8
#define ERR_CODE_INSTR   16

// The handler called by the asm handler
void vmm_page_fault_handler(vaddr_t addr, vaddr_t rip, uint64_t err) {
	struct ptable *pml4, *pdp, *pd, *pt;
	paddr_t paddr;
	uint64_t idx, flags;
	klog ("Page fault at %#llx, RIP: %#llx, ERR: %#llx\n", addr, rip, err);
	// If it's a no-exec fault, then abort
	if (err & ERR_CODE_INSTR) {
		klog ("Attempt to execute at no-exec memory at %#llx. Abort.\n", addr);
		crash_and_burn ();
	}
	// If it's a reserved bit detection fault, then abort
	if (err & ERR_CODE_RSVD) {
		klog ("Reserved bit set. Addr = %#llx. Abort\n", addr);
		crash_and_burn ();
	}
	// If it's not present, but was marked for allocation, allocate it
	if (!(err & ERR_CODE_PRESENT)) {
		pml4 = (struct ptable*) PML4_VADDR;
		if (!(pdp = _pt_child (pml4, PML4_IDX (addr)))
		    || !(pd = _pt_child (pdp, PDP_IDX (addr)))
		    || !(pt = _pt_child (pd, PD_IDX (addr)))) {
			klog ("Rogue pointer: %#llx. Abort\n", addr);
			crash_and_burn ();
		}
		idx = PT_IDX (addr);
		// TODO: Swapping
		if (PTE_TO_ALLOC (pt->e[idx])) {
			ASSERT ((paddr = _PMMGR->alloc ()) != PADDR_INVALID);
			flags = PTE_FLAGS (pt->e[idx]);
			flags = (flags | PTE_FLG_PRESENT) & ~PTE_FLG_TO_ALLOC;
			PTE_SET (pt->e[idx], paddr, flags);
			return;
		}
		klog ("Rogue pointer: %#llx. Abort\n", addr);
		crash_and_burn ();
	}
}

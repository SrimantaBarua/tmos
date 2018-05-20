// (C) 2018 Srimanta Barua

#include <memory.h>
#include <system.h>
#include <arch/x86_64/memory.h>

// TODO: Per-CPU lockless memory pools
// TODO: Global heap locking

// Initialize the memory management subsystem with the given underlying physical memory manager
// Also provide an optional callback for remapping in case required
void mem_init(struct pmmgr *pmmgr, void (*remap_cb) (void)) {
	// Initialize virtual memory manager
	vmm_init (pmmgr, remap_cb);
	// Initialize the heap
}

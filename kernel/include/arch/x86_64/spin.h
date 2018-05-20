// (C) 2018 Srimanta Barua
//
// Spinlocks

#pragma once

#include <system.h>

// A spinlock
typedef uint32_t spin_t;

// Initialize a spinlock
#define SPIN_UNLOCKED 0

#ifndef __CFG_SMP__

	// Acquire a spinlock
	FORCEINLINE void spin_lock(spin_t *lock) {
		if (sys_int_enabled ()) {
			sys_disable_int ();
			*lock = 1;
		}
	}

	// Acquire a spinlock, disabling interrupts when it has been acquired
	FORCEINLINE void spin_lock_intsafe(spin_t *lock) {
		spin_lock (lock);
	}

	// Release a spinlock. If interrupts were enabled before locking, enable interrupts
	FORCEINLINE void spin_unlock(spin_t *lock) {
		if (*lock) {
			*lock = 0;
			sys_enable_int ();
		}
	}

#else

	// Acquire a spinlock
	void spin_lock(spin_t *lock);

	// Acquire a spinlock, disabling interrupts when it has been acquired
	void spin_lock_intsafe(spin_t *lock);

	// Release a spinlock
	void spin_unlock(spin_t *lock);

#endif

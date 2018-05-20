// (C) 2018 Srimanta Barua
//
// Arch-neutral interface for spinlocks

#pragma once

#if defined(__CFG_ARCH_x86_64__)
#include <arch/x86_64/spin.h>
#endif

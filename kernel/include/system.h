// (C) 2018 Srimanta Barua

#pragma once

#include <stdint.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERISON_PATCH 0

#if defined(__ARCH_x86_64__)
#include <arch/x86_64/system.h>
#endif

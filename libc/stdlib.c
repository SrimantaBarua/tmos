// (C) 2018 Srimanta Barua

#include <stdlib.h>

// Cause normal process termination
void __attribute__((noreturn)) exit(int status) {
	while (1) { }
	__builtin_unreachable();
}

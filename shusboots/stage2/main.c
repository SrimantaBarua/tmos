// main.c
// Central point of the C code for stage2 of the bootloader
// (C) 2018 Srimanta Barua

#include <stdint.h>

void main() {
	volatile uint32_t *ptr = 0xb8000;
	*ptr = 0x0f4b0f4f;
	__asm__ __volatile__ ("cli; hlt; jmp $" : : : );
}

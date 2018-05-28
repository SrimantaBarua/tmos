// (C) 2018 Srimanta Barua
//
// Interface which lets us carry out port I/O from C

#pragma once

#include <stdint.h>
#include <system.h>

// Read a byte in from a port
static inline uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ __volatile__ ("inb al, dx" : "=a"(ret) : "d"(port));
	return ret;
}

// Write a byte out to a port
static inline void outb(uint16_t port, uint8_t data) {
	__asm__ __volatile__ ("outb dx, al" : : "a"(data), "d"(port));
}

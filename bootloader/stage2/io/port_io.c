// (C) 2018 Srimanta Barua
//
// Port I/O from C

#include <port_io.h>

// Read a byte in from a port
uint8_t inb(uint16_t port) {
	uint8_t ret;
	__asm__ __volatile__ ("inb al, dx" : "=a"(ret) : "d"(port));
	return ret;
}

// Write a byte out to a port
void outb(uint16_t port, uint8_t data) {
	__asm__ __volatile__ ("outb dx, al" : : "a"(data), "d"(port));
}

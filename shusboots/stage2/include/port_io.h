// Interface which lets us carry out port I/O from C

#pragma once

#include <stdint.h>

// Read a byte in from a port
uint8_t inb(uint16_t port);

// Write a byte out to a port
void outb(uint16_t port, uint8_t data);

// Interface for serial port I/O

#pragma once

#include <stdint.h>

// The serial port we want to talk on
enum serial_port {
	COM1 = 0x3f8,
	COM2 = 0x2f8,
	COM3 = 0x3e8,
	COM4 = 0x2e8,
};

// Initialize the serial port interface
// (Currently only COM1)
void serial_init();

// Write a byte on the given serial port
void serial_write_byte(enum serial_port port, uint8_t byte);

// Write a null-terminated string to the given serial port
//void serial_write_str(enum serial_port port, const char *str);
void serial_write_str(const char *str);

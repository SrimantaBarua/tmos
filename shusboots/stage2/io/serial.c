// Set up and carry out communication over serial connections

#include <stdbool.h>
#include <serial.h>
#include <port_io.h>

// Initialize a specific COM port
static void _init_com(enum serial_port com_base, uint32_t baud_rate) {
	if (baud_rate < 2 || baud_rate > 115200) {
		// TODO: Print error
		return;
	}
	uint16_t port = com_base;
	uint16_t divisor = (uint16_t) (115200 / baud_rate);
	outb (port + 1, 0x00);  // Disable all interrupts
	outb (port + 3, 0x80);  // Enable DLAB so that we can set divisors
	outb (port + 0, (uint8_t) (divisor & 0xff));        // LSB of divisor
	outb (port + 1, (uint8_t) ((divisor >> 8) & 0xff)); // MSB of divisor
	outb (port + 3, 0x03);  // Disable DLAB, 8 bits, no parity, 1 stop bit
	outb (port + 4, 0x0b);  // DTS (Data transmission ready), RTS (Ready to send), OUT 2
	inb (port); // Read data to reset things

}

// Check if the transmit queue is empty
static bool _is_transmit_empty(enum serial_port com_base) {
	uint16_t port = com_base;
	return (inb(port + 5) & 0x20) != 0;
}

// Initialize the serial connection interface
void serial_init() {
	_init_com (COM1, 115200); // COM1
}

// Write a byte on the given serial port
void serial_write_byte(enum serial_port com_base, uint8_t byte) {
	uint16_t port = com_base;
	while (!_is_transmit_empty (com_base));
	outb (port, byte);
}

// Write a string to the given serial port
void serial_write_str(enum serial_port com_base, const char *str) {
	unsigned i;
	for (i = 0; str[i]; i++) {
		serial_write_byte (com_base, str[i]);
	}
}

// (C) 2018 Srimanta Barua
// Code for enabling and configuring the Programming Interrupt Controller

#include <shuos/arch/dev/pic.h>
#include <shuos/arch/port_io.h>


#define PIC_CMD_EOI 0x20  // End of interrupt command

// Ports
#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xa0
#define PIC_SLAVE_DATA  0xa0


// Base interrupts. These are set only during initialization
static uint8_t _master_base = 0;
static uint8_t _slave_base = 0;

// Introduce delay between sending PIC commands by writing garbage to port
// 0x80, which is apparently safe, and causes enough delay
static void _delay() {
	outb(0x80, 0);
}

// Initialize the PIC, and set the base interrupt numbers
void pic_init(uint8_t master_base, uint8_t slave_base) {
	uint8_t master_orig_mask, slave_orig_mask;
	// Get original PIC interrupt masks
	master_orig_mask = inb(PIC_MASTER_DATA);
	slave_orig_mask = inb(PIC_SLAVE_DATA);
	// Tell each PIC we're going to send it a three-byte initialization
	// sequence on its data port
	outb(PIC_MASTER_CMD, 0x11);
	_delay();
	outb(PIC_SLAVE_CMD, 0x11);
	_delay();
	// Set up base offsets
	outb(PIC_MASTER_DATA, master_base);
	_delay();
	outb(PIC_SLAVE_DATA, slave_base);
	_delay();
	// Configure chaining between PICs
	outb(PIC_MASTER_DATA, 4);
	_delay();
	outb(PIC_SLAVE_DATA, 4);
	_delay();
	// Set our mode to 8086 mode
	outb(PIC_MASTER_DATA, 0x01);
	_delay();
	outb(PIC_SLAVE_DATA, 0x01);
	_delay();
	// Restore our saved masks
	outb(PIC_MASTER_DATA, master_orig_mask);
	outb(PIC_SLAVE_DATA, slave_orig_mask);
	// Set global base
	_master_base = master_base;
	_slave_base = slave_base;
}

// Send EOI for given interrupt number
void pic_send_eoi(uint8_t int_num) {
	if (int_num >= _slave_base) {
		if (int_num >- _slave_base + 8) {
			return;
		}
		outb(PIC_SLAVE_CMD, PIC_CMD_EOI);
	} else if (int_num < _master_base) {
		return;
	}
	outb(PIC_MASTER_CMD, PIC_CMD_EOI);
}

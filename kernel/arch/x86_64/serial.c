// (C) 2018 Srimanta Barua
//
// Set up and carry out communication over serial connections

#include <stdbool.h>
#include <shuos/serial.h>
#include <shuos/arch/x86_64/port_io.h>

// Registers
#define DATA(base)         (base)
#define INT_EN(base)       ((base) + 1)
#define FIFO_CTRL(base)    ((base) + 2)
#define LINE_CTRL(base)    ((base) + 3)
#define MODEM_CTRL(base)   ((base) + 4)
#define LINE_STATUS(base)  ((base) + 5)
#define MODEM_STATUS(base) ((base) + 6)
#define SCRATCH(base)      ((base) + 7)

// Interrupt bits
#define INT_DATA_AVAIL   1
#define INT_TRANS_EMPTY  2
#define INT_BREAK_ERR    4
#define INT_STATUS_CHNG  8

// Line control bits
#define LINE_CTRL_5BIT         0
#define LINE_CTRL_6BIT         1
#define LINE_CTRL_7BIT         2
#define LINE_CTRL_8BIT         3
#define LINE_CTRL_1STOP        0
#define LINE_CTRL_2STOP        (1 << 2)
#define LINE_CTRL_NO_PARITY    0
#define LINE_CTRL_ODD_PARITY   (1 << 3)
#define LINE_CTRL_EVEN_PARITY  (3 << 3)
#define LINE_CTRL_MARK_PARITY  (5 << 3)
#define LINE_CTRL_SPACE_PARITY (7 << 3)
#define LINE_CTRL_BREAK_ENABLE (1 << 6)
#define LINE_CTRL_DLAB         (1 << 7)

// Modem control bits
#define MODEM_CTRL_DTR       (1 << 0)
#define MODEM_CTRL_RTS       (1 << 1)
#define MODEM_CTRL_OUT1      (1 << 2)
#define MODEM_CTRL_OUT2      (1 << 3)
#define MODEM_CTRL_LOOPBACK  (1 << 4)

// Line status bits
#define LINE_STATUS_DATA_READY      (1 << 0)
#define LINE_STATUS_OVERRUN_ERR     (1 << 1)
#define LINE_STATUS_PARITY_ERR      (1 << 2)
#define LINE_STATUS_FRAMING_ERR     (1 << 3)
#define LINE_STATUS_BREAK_INT       (1 << 4)
#define LINE_STATUS_TRANS_EMPTY     (1 << 5)
#define LINE_STATUS_DATA_HOLD_EMPTY (1 << 6)
#define LINE_STATUS_FIFO_RECV_ERR   (1 << 7)


// Initialize a specific COM port
static void _init_com(enum serial_port com_base, uint32_t baud_rate) {
	if (baud_rate < 2 || baud_rate > 115200) {
		// TODO: Print error
		return;
	}
	uint16_t port = com_base;
	uint16_t divisor = (uint16_t) (115200 / baud_rate);

	outb(INT_EN(port), 0x00);

	outb(LINE_CTRL(port), LINE_CTRL_DLAB);
	outb(DATA(port), (uint8_t) (divisor & 0xff));        
	outb(INT_EN(port), (uint8_t) ((divisor >> 8) & 0xff));
	outb(LINE_CTRL(port), LINE_CTRL_8BIT | LINE_CTRL_1STOP);

	outb(MODEM_CTRL(port), MODEM_CTRL_DTR | MODEM_CTRL_RTS | MODEM_CTRL_OUT2);

	inb(port); // Read data to reset things

}

// Check if the transmit queue is empty
static bool _is_transmit_empty(enum serial_port com_base) {
	uint16_t port = com_base;
	return (inb(LINE_STATUS(port)) & LINE_STATUS_TRANS_EMPTY) != 0;
}

// Initialize the serial connection interface
void serial_init() {
	_init_com(COM1, 115200); // COM1
}

// Write a byte on the given serial port
void serial_write_byte(enum serial_port com_base, uint8_t byte) {
	uint16_t port = com_base;
	while (!_is_transmit_empty(com_base));
	outb(port, byte);
}

// Write a string to the given serial port
void serial_write_str(enum serial_port com_base, const char *str) {
	if (str) {
		while (*str) {
			serial_write_byte(com_base, (uint8_t) *str);
			str++;
		}
	}
}


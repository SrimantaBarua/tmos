// (C) 2018 Srimanta Barua
// Code for configuring the Programmable Interval Timer, and for the PIT
// interrupt handler

#include <shuos/arch/dev/pit.h>
#include <shuos/arch/dev/pic.h>
#include <shuos/arch/port_io.h>
#include <shuos/arch/idt.h>
#include <shuos/klog.h>

// PIT ports
#define PIT_CHANNEL0  0x40
#define PIT_CHANNEL1  0x41
#define PIT_CHANNEL2  0x42
#define PIT_COMMAND   0x43


// Bitmap for the mode/command register at 0x43
#define PIT_CMD_BCD    0x01    // If 1, 4 digit BCD, else 16-bit binary

#define PIT_CMD_MODE0  0x00    // Interrupt on terminal count
#define PIT_CMD_MODE1  0x02    // Hardware re-triggerable one-shot
#define PIT_CMD_MODE2  0x04    // Rate generator
#define PIT_CMD_MODE3  0x06    // Square wave generator
#define PIT_CMD_MODE4  0x08    // Software-triggered strobe
#define PIT_CMD_MODE5  0x0A    // Hardware-triggered strobe

#define PIT_CMD_LATCH  0x00
#define PIT_CMD_LOBYTE 0x10
#define PIT_CMD_HIBYTE 0x20
#define PIT_CMD_RW16   0x30

#define PIT_CMD_CHANNEL0  0x00
#define PIT_CMD_CHANNEL1  0x40
#define PIT_CMD_CHANNEL2  0x80
#define PIT_CMD_READ_BACK 0xC0

#define PIT_FREQ 1193182


// Static store of number of PIT ticks
static uint64_t _pit_ticks;

// Helper for PIT IRQ
static void __attribute__((used)) _isr_pit_helper() {
	_pit_ticks++;
	pic_send_eoi(IRQ_TIMER);
}

// Interrupt handler for PIT ticks
static void __attribute__((naked)) _isr_pit() {
	__asm__ __volatile__("call _isr_pit_helper; iretq");
}

// Configure the PIT for a continuous timer tick, with given frequency (Hz)
void pit_start_counter(uint32_t freq) {
	uint16_t div;
	ASSERT(freq > 0);
	ASSERT(freq <= PIT_FREQ);
	isr_set_gate(IRQ_TIMER, _isr_pit, 0, 0x08, IDT_ATTR_PRESENT | IDT_ATTR_INT_32);
	div = PIT_FREQ / freq;
	outb(PIT_COMMAND, PIT_CMD_RW16 | PIT_CMD_MODE2 | PIT_CMD_CHANNEL0);
	outb(PIT_CHANNEL2, div & 0xff);
	outb(PIT_CHANNEL2, (div >> 8) & 0xff);
	_pit_ticks = 0;
}

// Get number of ticks
uint64_t pit_get_ticks() {
	return _pit_ticks;
}

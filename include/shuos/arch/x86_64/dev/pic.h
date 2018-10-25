// (C) 2018 Srimanta Barua
// Interface for the Programmable Interrupt Conttoller

#pragma once

#include <stdint.h>

// Initialize the PIC, and set the base interrupt numbers
void pic_init(uint8_t master_base, uint8_t slave_base);

// Send EOI for given interrupt number
void pic_send_eoi(uint8_t int_num);

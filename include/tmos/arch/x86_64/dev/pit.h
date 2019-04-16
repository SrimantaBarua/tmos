// (C) 2018 Srimanta Barua
// Interface for the Programmable Interval Timer

#pragma once

#include  <stdint.h>

// Configure the PIT for a continuous timer tick, with given frequency (Hz)
void pit_start_counter(uint32_t freq);

// Get number of ticks
uint64_t pit_get_ticks();

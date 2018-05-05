// (C) 2018 Srimanta Barua

#pragma once

#include <arch/x86_64/cpu.h>
#include <arch/x86_64/idt.h>

// Check if interrupts are enabled
#define sys_int_enabled() (cpu_read_rflags ().f.IF == 1)

// Disable interrupts
#define sys_disable_int() idt_disable_int ()

// Enable interrupts
#define sys_enable_int() idt_enable_int ()

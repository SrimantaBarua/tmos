; entry.asm
; Entry point for stage2 of the bootloader
; (C) 2018 Srimanta Barua
;
; This just jumps to the beginning of C code. So it HAS TO be at the beginning of the binary
; This file can also be a convenient place to put Assembly routines later..

[BITS 32]

section .stage2_start_section

global stage2_start
stage2_start:
	extern	main
	jmp	main

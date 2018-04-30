; (C) 2018 Srimanta Barua
;
; Entry point for when the kernel is loaded by a multiboot2-compliant bootloader. The multiboot2
; specification states that at this point, the state of the system would be -
; - EAX = 0x36d76289
; - EBX = pointer to multiboot2 information table
; - 32-bit protected mode
; - Flat addressing (effectively no segmentation)
; - Interrupts disabled
;
; This file checks for the availability of long mode, sets up paging, switches to long mode, and
; jumps to C code

[BITS 32]

; MULTIBOOT2 HEADER

section .multiboot2_header

mb2hdr:
	dd	0xe85250d6	; Magic number
	dd	0		; Arch 0 = protected mode i386
	dd	(.end - mb2hdr)	; Size of header
	dd	(0x100000000 - (0xe85250d6 + 0 + (.end - mb2hdr)))

	; Optional multiboot2 tags

	; Required end tag
	align 8
	dw	0	; Type = end tag
	dw	0	; Flags = non-optional
	dd	8	; Size
.end:


; MULTIBOOT2 ENTRY POINT

section .bootstrap.x86

global _start
_start:
	cli
	mov	esp, bootstrap_stack_top
	push	ebx

	call	check_multiboot2
	call	check_cpuid
	call	check_long_mode

	mov	dword [0xb8000], 0x2f4b2f4f

halt:
	cli
	hlt
	jmp	halt


; Check if we indeed were loaded by a multiboot2-compliant bootloader. The bootloader would have
; put the value 0x36d76289 in EAX
check_multiboot2:
	cmp	eax, 0x36d76289
	jne	halt
	ret

; Check if CPUID is available, by trying to flip the ID bit (bit 21) of the EFLAGS register. If it
; can be flipped, CPUID is available
check_cpuid:
	pushfd
	pop	eax
	mov	ecx, eax
	xor	eax, 1 << 21
	push	eax
	popfd
	pushfd
	pop	eax
	push	ecx
	popfd
	cmp	eax, ecx
	je	halt
	ret

; Check if CPUID function 0x80000001 is available. If yes, call that function and check if the
; Long Mode bit (bit 29 in EDX) is set. If yes, long mode is available
check_long_mode:
	mov	eax, 0x80000000
	cpuid
	cmp	eax, 0x80000001
	jb	halt
	mov	eax, 0x80000001
	cpuid
	test	edx, 1 << 29
	jz	halt
	ret

; A tiny, initial 128-byte stack
align 8
bootstrap_stack_bottom:
	times 128 db 0
bootstrap_stack_top:

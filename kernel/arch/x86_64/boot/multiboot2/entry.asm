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

	; A few checks
	call	check_multiboot2
	call	check_cpuid
	call	check_long_mode

	; Set up and enable paging and long mode
	call	set_up_page_tables
	call	enable_paging

	pop	ebx

	; Load a valid 64-bit GDT and jump to long mode entry
	lgdt	[gdtr64]
	jmp	gdt64.code:long_mode_init

halt:
	cli
	hlt
	jmp	halt

; A tiny, initial 128-byte stack
align 8
bootstrap_stack_bottom:
	times 128 db 0
bootstrap_stack_top:

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

; Set up page tables. Set them up such that they look something like this -
; pml4[0]   -> pdp0
;     pdp0[0] -> 0 - 1 GB (Huge page)
; pml4[510] -> pml4 (recursive mapping)
; pml4[511] -> pdp1
;     pdp1[510] -> 0 - 1 GB (Huge page)
;
; This results in the first 2 MB of RAM being identity-mapped using one huge page, and also
; mapped from a higher address (-2GB).
set_up_page_tables:
	mov	eax, pdp0		; PML4[0] -> PDP0
	or	eax, 0x03		; Present + writable
	mov	[pml4], eax
	mov	eax, pml4		; PML4[510] -> PML4
	mov	[pml4 + 0xff0], eax
	or	eax, 0x03
	mov	eax, pdp1		; PML4[511] -> PDP1
	or	eax, 0x03
	mov	[pml4 + 0xff8], eax
	xor	eax, eax		; PDP0[0] -> 0 - 1 GB | PDP1[510] -> 0 - 1 GB
	or	eax, 0x83		; Present + writable + huge
	mov	[pdp0], eax
	mov	[pdp1 + 0xff0], eax
	ret

; Enable paging and long mode
enable_paging:
	; Load PML4 into CR3 register
	mov	eax, pml4
	mov	cr3, eax
	; Enable PAE (Physical address extension) in CR4
	mov	eax, cr4
	or	eax, 1 << 5
	mov	cr4, eax
	; Set the long mode bit in the EFER MSR
	mov	ecx, 0xc0000080
	rdmsr
	or	eax, 1 << 8
	wrmsr
	; Enable paging in  CR0
	mov	eax, cr0
	or	eax, 1 << 31
	mov	cr0, eax
	ret

; 64-bit GDT
gdt64:
    .null: equ ($ - gdt64)			; NULL descriptor
	dq 0
    .code: equ ($ - gdt64)			; Code descriptor
	dq (1 << 41) | (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53)
    .data: equ ($ - gdt64)			; Data descriptor
	dq (1 << 41) | (1 << 44) | (1 << 47)

; GDTR
gdtr64:
	.limit:	dw ($ - gdt64) - 1
	.base:  dq gdt64

align 4096
pml4:
	times 4096 db 0
pdp0:
	times 4096 db 0
pdp1:
	times 4096 db 0



; LONG MODE ENTRY POINT

[BITS 64]

section .bootstrap.x86_64

long_mode_init:
	mov	ax, gdt64.data
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Jump to C kernel code
	xor	rdi, rdi
	mov	edi, ebx
	extern	kinit_multiboot2
	call	kinit_multiboot2

	cli
	hlt
	jmp	$

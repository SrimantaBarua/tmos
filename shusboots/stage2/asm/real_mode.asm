; (C) 2019 Srimanta Barua
;
; BIOS interrupts are useful. Atleast for a bootloader, I guess it's okay to use them.
; This file contains an entry-point which switches to 16-bit real mode, and then runs a
; function which was passed to it as a parameter.


[BITS 32]


extern halt_32
extern vlog
extern log


LOG_INFO:     equ 0
LOG_WARN:     equ 1
LOG_ERR:      equ 2
LOG_SUCCESS:  equ 3


section .text


; Switch to real mode and run a function
; Params - linear pointer to function, number of args, variable arguments
; Clobbers - AX
global real_call
real_call:
	; Store registers
	push	ebp
	mov	ebp, esp
	pushfd
	push	ebx
	push	edx
	push	edi
	push	esi

	; Verify pointer is <= 0xffff
	mov	eax, [ebp + 8]
	cmp	eax, 0xffff
	jg	.ptr_too_high

	; Verify pointer is not NULL
	or	eax, eax
	jz	.ptr_null

	; Store pointer
	mov	edx, eax

	; Disable NMI
	in	al, 0x70
	or	al, 0x80
	out	0x70, al

	; Far jump to 16-bit protected mode
	jmp	0x18:protected_16_mode

.ptr_too_high:
	and	esp, 0xfffffff0
	sub	esp, 4
	push	eax
	mov	eax, msg_ptr_too_high
	push	eax
	mov	eax, LOG_ERR
	push	eax
	call	log
	jmp	halt_32

.ptr_null:
	and	esp, 0xfffffff0
	sub	esp, 8
	mov	eax, msg_ptr_null
	push	eax
	mov	eax, LOG_ERR
	push	eax
	call	log
	jmp	halt_32



[BITS 16]


; 16-bit protected mode. Make the switch to real mode and pass parameters to functions
protected_16_mode:
	; Disable interrupts
	cli
	; Load 16-bit entries
	mov	ax, 0x20
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax
	; Disable paging and protected mode
	mov	eax, cr0
	mov	[save_cr0], eax
	and	eax, 0x7ffffffe    ; Disable paging and protected mode
	mov	cr0, eax
	; Go to real mode 
	jmp	0:real_mode_start



; 16-bit real mode
; Convert arguments to 16-bit, and run real mode function
real_mode_start:
	; Set segment registers
	xor	ax, ax
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Enable NMI
	in	al, 0x70
	and	al, 0x7e
	out	0x70, al

	; Load real-mode IDT
	lidt	[idt_real]

	; Enable interrupts
	sti

	; Convert args to 16-bit
	mov	cx, [bp + 12]  ; Number of arguments
	mov	bx, cx

	; Source and destination for args
	lea	si, [bp + 16]
	mov	di, sp
	sub	di, cx
	sub	di, cx

	; Loop start
	or	cx, cx
	jz	.loop_done

	cld

.loop:
	movsw
	add	si, 2
	loop	.loop

.loop_done:
	sub	sp, bx
	sub	sp, bx

	; Call the function
	call	dx

	; Reset stack
	add	sp, bx
	add	sp, bx

	; Store return value
	xor	ebx, ebx
	mov	ebx, eax


; Exit real mode
real_mode_end:
	; Disable interrupts
	cli

	; Disable NMI
	in	al, 0x70
	or	al, 0x80
	out	0x70, al

	; Load original CR0
	mov	eax, [save_cr0]
	mov	cr0, eax

	; Jump to 32-bit protected mode
	jmp	0x08:protected_mode_reentry


[BITS 32]

; Re-enter 32-bit protected mode
protected_mode_reentry:
	mov	ax, 0x10
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

	; Enable NMI
	in	al, 0x70
	and	al, 0x7e
	out	0x70, al

	; Restore return value
	mov	eax, ebx

	; Reset stack
	pop	esi
	pop	edi
	pop	edx
	pop	ebx
	popfd
	pop	ebp

	; Return
	ret



align 2
idt_real:
	dw	0x3ff  ; 256 entries, 4b each = 1K
	dd	0      ; Real mode IVT is at 0



section .data

save_cr0:  dd 0


section .rodata

; Strings
msg_ptr_too_high: db "Pointer too high: 0x%x", 0x0a, 0x00
msg_ptr_null:     db "Pointer is NULL", 0x0a, 0x00

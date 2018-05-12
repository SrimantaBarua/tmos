; (C) 2018 Srimanta Barua
; The page fault ISR

global isr_page_fault

[BITS 64]

%macro PUSH_ALL 0
	push rax
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
%endmacro

%macro POP_ALL 0
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx
	pop rax
%endmacro

section .text.isr_page_fault
isr_page_fault:
	; Push scratch registers
	PUSH_ALL

	; Parameters to handler - CR2 (fault address), IP and error code
	mov	rdi, cr2               ; Fault address
	mov	rsi, qword [rsp + 80]  ; IP
	mov	rdx, qword [rsp + 72]  ; Error code

	; Call handler
	extern	vmm_page_fault_handler
	call	vmm_page_fault_handler

	POP_ALL

	add rsp, 8 ; Remove error code
	iretq

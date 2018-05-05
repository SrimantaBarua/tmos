; (C) 2018 Srimanta Barua

[BITS 64]

global cpu_read_rflags
global cpu_write_rflags

global idt_disable_int
global idt_enable_int

global crash_and_burn

; Read the CPU's RFLAGS register
; Returns -> AX = value
section .text.cpu_read_rflags
cpu_read_rflags:
	pushfq
	pop	rax

; Write the CPU's RFLAGS register
; Params -> RDI = value to write
section .text.cpu_write_rflags
cpu_write_rflags:
	push	rdi
	popfq

; Enable interrupts
section .text.idt_enable_int
idt_enable_int:
	sti
	ret

; Disable interrupts
section .text.idt_enable_int
idt_disable_int:
	cli
	ret

; Stop forever
section .text.crash_and_burn
crash_and_burn:
	cli
	hlt
	jmp	crash_and_burn

; -------------------------------
; CPU Interrupt Service Routines
; -------------------------------

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

; Exception vectors which do not push error codes
%macro EXCEPT_NOERROR 1
global isr_%1
isr_%1:
	push 0			; Dummy error code
	push %1 		; Exception number
	jmp  _isr_common  	; Jump to common ISR handler
%endmacro

; Exception vectors which push error codes
%macro EXCEPT_ERROR 1
global isr_%1
isr_%1:
	push %1 		; Exception number
	jmp  _isr_common  	; Jump to common ISR handler
%endmacro


; Commong part of known exception handlers
section .text._isr_common
_isr_common:
	PUSH_ALL			; Store registers
	mov	rdi, rsp		; Pass pointers to pushed registers
	extern	isr_common_c_handler
	call	isr_common_c_handler	; Jump to common C handler

section .text

; ISRs
EXCEPT_NOERROR 0
EXCEPT_NOERROR 1
EXCEPT_NOERROR 2
EXCEPT_NOERROR 3
EXCEPT_NOERROR 4
EXCEPT_NOERROR 5
EXCEPT_NOERROR 6
EXCEPT_NOERROR 7
EXCEPT_ERROR 8
EXCEPT_NOERROR 9
EXCEPT_ERROR 10
EXCEPT_ERROR 11
EXCEPT_ERROR 12
EXCEPT_ERROR 13
EXCEPT_ERROR 14
EXCEPT_NOERROR 16
EXCEPT_ERROR 17
EXCEPT_NOERROR 18
EXCEPT_NOERROR 19
EXCEPT_NOERROR 20


; (C) 2018 Srimanta Barua

[BITS 64]

global set_cs

; Set the CS segment register
section .text.set_cs
set_cs:
	and	rdi, 0xffff
	xor	rax, rax
	mov	ax, ss
	mov	rdx, rsp
	mov	rcx, .loc

	push	rax
	push	rdx
	pushf
	push	rdi
	push	rcx
	iretq

.loc:
	ret

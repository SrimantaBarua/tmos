; (C) 2018 Srimanta Barua
;
; crt0.o will contain the _start function, which is the entry point for all
; user-mode programs. _start initialized the process, calls main(), and then
; calls exit().

[BITS  64]

section .text

extern _init
extern main
extern exit

global _start
_start:
	; Set up the end of the stack frame linked list. This helps in
	; debugging, when we wish to browse stack frames, since this puts an
	; explicit 0 at the beginning
	mov  rbp, 0
	push rbp      ; RIP = 0
	push rbp      ; RBP = 0
	mov  rbp, rsp
	; Store argc, argv, envp on the stack
	push rdi
	push rsi
	push rdx
	; Prepare stdlib features like memory management, signals, stdio etc
	; TODO
	; Run the global constructors
	call _init
	; Restore argc, argv, envp
	pop  rdx
	pop  rsi
	pop  rdi
	; Run main
	call main
	; Terminate the process with exit code
	mov  edi, eax
	call exit

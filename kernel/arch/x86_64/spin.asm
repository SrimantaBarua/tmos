; (C) 2018 Srimanta Barua
; Spinlocks for SMP systems

[BITS 64]

global spin_lock
global spin_lock_intsafe
global spin_unlock


section .text.spin_lock
; Params - RDI = spin_t *lock
spin_lock:
	lock bts dword [rdi], 0  ; Optimistically set bit 0 and return previous value in CF
	jnc	.acquired        ; Lock was previously 0, so acquired
.retry:
	pause                    ; Don't waste CPU resources
	bt	dword [rdi], 0   ; Check if bit 0 is set
	jc	.retry           ; Yes, poll again
	lock bts dword [rdi], 0  ; Set bit 0 and return previous value in CF
	jc	.retry           ; Retry if bit 0 was set
.acquired:
	ret


section .text.spin_lock_intsafe
; Params - RDI = spin_t *lock
spin_lock_intsafe:
	; Store original flags
	pushf
	pop	rax

	cli                      ; Disable IRQs in the hope of acquiring lock
	lock bts dword [rdi], 0  ; Optimistically set bit 0 and return previous value in CF
	jnc	.acquired        ; Lock was previously 0, so acquired
.retry:
	sti                      ; Enable IRQs for polling
	pause                    ; Don't waste CPU resources
	bt	dword [rdi], 0   ; Check if bit 0 is set
	jc	.retry           ; Yes, poll again
	cli                      ; Disable IRQs in the hope of acquiring lock
	lock bts dword [rdi], 0  ; Set bit 0 and return previous value in CF
	jc	.retry           ; Retry if bit 0 was set
.acquired:
	; Were interrupts enabled originally?
	test	ah, 2
	jz	.done            ; No, return
	and	dword [rdi], 2   ; Set bit 1 to denote that interrupts were enabled
.done:
	ret

section .text.spin_unlock
; Params - RDI = spin_t *lock
spin_unlock:
	; Check if interrupts were enabled before locking
	test	dword [rdi], 2
	jz	.not_enabled
.enabled:
	sti
.not_enabled:
	mov	dword [rdi], 0
	ret

; Entry point of the stage2 binary. This HAS TO be at the exact beginning of the image. It does
; nothing other than jumping to Rust code.
;
; This might also later be a convenient place to put other ASM code we might need..

[BITS 32]

section .stage2_start_section

; Start point. This HAS TO be at the beginning of the image
global stage2_start
stage2_start:
	extern	rust_main
	call	rust_main

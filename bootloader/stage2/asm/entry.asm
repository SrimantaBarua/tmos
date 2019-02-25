; entry.asm
; Entry point for stage2 of the bootloader
; (C) 2018 Srimanta Barua
;
; This just jumps to the beginning of C code. So it HAS TO be at the beginning of the binary
; This file can also be a convenient place to put Assembly routines later..


MEM_MAP_SEG:        equ 0x1000
MEM_MAP_BASE:       equ 0x10000


[BITS 16]


section .stage2_start_section


; Start of the stage 2 bootloader. This loads C code, makes the switch to 32-bit protected mode,
; and jumps to the C code. It also does the following things -
; - Reads the BIOS memory map
; - Gives user a menu to select display mode. (As of now, this mode will stick till next reset)
global stage2_start
stage2_start:
	; Read BIOS memory map to 0x1000:0x0000 (0x10000)
	mov	ax, MEM_MAP_SEG
	mov	es, ax
	mov	di, 8
	call	int_15_e820_read_mmap
	or	ax, ax
	jnz	.mem_map_ok

.mem_map_err:
	mov	ds, ax
	mov	si, msg_mem_map_err
	call	print_bios_16
	jmp	halt_16

.mem_map_ok:
	; Store number of entries
	mov	cx, 0x1000
	mov	ds, cx
	mov	word [ds:0], ax
	mov	word [ds:2], 0
	mov	word [ds:4], 0
	mov	word [ds:6], 0
	xor	ax, ax
	mov	ds, ax
	mov	si, msg_mem_map_ok
	call	print_bios_16


; Go to protected mode
; For that, we need to -
;  - Disable interrupts and NMI
;  - Enable the A20 address line
;  - Load a valid GDT
;  - Set bit 0 in CR0
;  - Jump to set new CS
go_to_pmode:
	cli

	; Disable NMI
	in	al, 0x70
	or	al, 0x80
	out	0x70, al

	; Enable A20 address line
	call	a20_enable
	or	al, al
	jz	.no_a20

	; Load valid GDT
	lgdt	[gdtr32]

	; Set bit 0 in CR0
	mov	eax, cr0
	or	al, 1
	mov	cr0, eax

	; Jump to pmode
	jmp	gdt32.code32:protected_mode_start

.no_a20:
	xor	ax, ax
	mov	si, msg_no_a20
	call	print_bios_16
	jmp	halt_16



section .text


; Print a '\0' - terminated string using BIOS interrupt 0x10
; This interrupt requires that -
;     AH = 0x0E
;     AL = <ascii char to print>
; Params -  Pointer to string in DS:SI
; Returns - AX = number of bytes written
; Clobbers - SI, Flags
print_bios_16:
	push	cx
	xor	cx, cx
.loop:
	lodsb
	or	al, al
	jz	.done
	mov	ah, 0x0e
	int	0x10
	inc	cx
	jmp	.loop
.done:
	mov	ax, cx
	pop	cx
	ret


; 16-bit code for halting
halt_16:
	cli
	hlt
	jmp	halt_16


; Read memory maps using BIOS interrupt 0x15, function 0xE820
; This reads a (probably) unsorted array of memory map entries, into the buffer pointed to by
; ES:DI. It also returns the number of entries in AX, or 0 on error
; The structure of a memory map entry looks like -
;
; struct mmap_entry {
;     uint64_t base_addr;  // Start address of region
;     uint64_t length;     // Length of region in bytes
;     uint32_t type;       // Type of region (explained below)
;     uint32_t acpi;       // ACPI extended attributes (explained below)
; };
;
; Known values of the `type` field above -
;    1 = Usable (normal) RAM
;    2 = Reserved (unusable)
;    3 = ACPI reclaimable memory
;    4 = ACPI non-volatile storage memory
;    5 = Area containing bad memory
; In case `type` has any other value, it's probably safest to assume it to be 2 (reserved)
;
; ACPI extended attributes is basically pointless. Except -
;   Bit 0 = If clear, entire entry should be ignored
;   Bit 1 = If set, entry is non-volatile
; The other 30 bits are undefined (atleast according to the OS-Dev Wiki)
;
; This code is slightly modified from the OS-Dev Wiki article on detecting memory on x86
; (https://wiki.osdev.org/Detecting_Memory_(x86))
;
; Params  - ES:DI = pointer to buffer for memory map array
; Returns - Number of entries in AX, or 0 on error
int_15_e820_read_mmap:
	; Store registers
	push	bp
	push	bx
	push	cx
	push	dx
	push	di
	push	es

	xor	ebx, ebx		; EBX must be 0 to start
	xor	bp, bp			; Use BP to count
	mov	edx, 0x534d4150		; Put 'SMAP' in EDX
	mov	eax, 0xe820		; Required for the interrupt
	mov	[es:di + 20], dword 1	; Force a valid ACPI 3.x entry
	mov	ecx, 24			; Ask for 24 bytes
	int	0x15
	jc	.failed			; If carry is set on first call, unsupported
	mov	edx, 0x534d4150		; Put 'SMAP' in EDX for subsequent calls
	cmp	eax, edx		; Check if EAX has 'SMAP' after int
	jne	.failed			; If not, fail
	or	ebx, ebx		; EBX=0 implies list is 1 entry long. Worthless
	jz	.failed
	jmp	.jmp_in
.e820lp:
	mov	eax, 0xe820		; Required for the interrupt
	mov	[es:di + 20], dword 1	; Force a valid ACPI 3.x entry
	mov	ecx, 24			; Ask for 24 bytes
	int	0x15
	jc	.done			; Carry set means 'end of list already reached'
	mov	edx, 0x534d4150		; Put 'SMAP' in EDX for subsequent calls
.jmp_in:
	or	cx, cx			; Skip any zero length entries
	jz	.skip_ent
	cmp	cl, 20			; Check if response is 24 bit
	jbe	.not_ext		; No. Move to processing
	test	byte [es:di + 20], 1	; Is 'ignore data bit' clear?
	jz	.skip_ent		; Yes, skip
.not_ext:
	mov	ecx, [es:di + 8]	; Get lower uint32_t of address
	or	ecx, [es:di + 12]	; Or with upper uint32_t to check if address is 0
	jz	.skip_ent		; If yes, check
	inc	bp			; Increment count
	add	di, 24			; Move pointer to next 24 bytes
	jnc	.skip_ent		; No overflow, go to next entry
	mov	ax, es			; Overflow. Add to ES
	add	ax, 0x1000
	mov	es, ax
.skip_ent:
	or	ebx, ebx		; If EBX resets to 0, then list is complete
	jnz	.e820lp
.done:
	mov	ax, bp			; Get count in ax
	; Restore registers
	pop	es
	pop	di
	pop	dx
	pop	cx
	pop	bx
	pop	bp
	ret
.failed:
	xor	bp, bp			; Zero count to denote error
	jmp	.done


; Enable the A20 line. Some BIOSes enable this by default. Plus, not all methods work on all
; BIOSes. So, the OS-Dev Wiki recommends that we go about this in the following order -
; - Test is A20 line is enabled
; - If no, try using the BIOS function
; - Test is A20 line is enabled
; - If no, try using the keyboard controller method
; - Test is A20 line is enabled
; - If no, try using the fast A20 method
; - Test is A20 line is enabled
; - If no, print error message and halt
; All the code related to testing/enabling the A20 line has been modified from the OS-Dev Wiki
; article on A20 line (https://wiki.osdev.org/A20_Line)
;
; Returns - AX = 1 if enabled, 0 if not
a20_enable:
	call	test_a20	; Check if already enabled
	or	al, al
	jnz	.done
	call	a20_bios_int_15	; Try BIOS int 15h method
	call	test_a20	; Check if enabled
	or	al, al
	jnz	.done
	call	a20_keyboard	; Try keyboard controller method
	call	test_a20	; Check if enabled
	or	al, al
	jnz	.done
	call	a20_fast	; Try fast A20 method
	call	test_a20	; Check if enabled
.done:
	ret

; Enable A20 line using BIOS interrupt 0x15. The OS-Dev wiki recommends that we ignore the
; return status
a20_bios_int_15:
	mov	ax, 0x2403	; Check for support
	int	0x15
	jc	.done
	or	ah, ah
	jnz	.done
	mov	ax, 0x2402	; Get status
	int	0x15
	jc	.done
	or	ah, ah
	jnz	.done
	cmp	al, 1		; Check if already activated
	je	.done
	mov	ax, 0x2401	; Activate A20 gate
	int	0x15
.done:
	ret

; Enable A20 using the keyboard controller chip (8042)
; Port 0x64 is used to read the status register, or send a command to the controller
; Port 0x60 is used to write to the input buffer, or read the output buffer
a20_keyboard:
	call	.wait_input	; Wait for input buffer to be emptied
	mov	al, 0xad	; Disable keyboard
	out	0x64, al

	call	.wait_input	; Wait for input buffer to be emptied
	mov	al, 0xd0	; Read output port
	out	0x64, al

	call	.wait_output	; Wait for output buffer to be filled
	in	al, 0x60	; Read output buffer
	push	ax

	call	.wait_input	; Wait for input buffer to be emptied
	mov	al, 0xd1	; Write output port
	out	0x64, al

	call	.wait_input	; Wait for input buffer to be emptied
	pop	ax
	or	al, 2		; Set bit 1 (enabled A20)
	out	0x60, al	; Write output port

	call	.wait_input	; Wait for input buffer to be emptied
	mov	al, 0xae	; Enable keyboard
	out	0x64, al

	call	.wait_input	; Wait for input buffer to be emptied
	ret

.wait_input:
	in	al, 0x64
	test	al, 2
	jnz	.wait_input
	ret

.wait_output:
	in	al, 0x64
	test	al, 1
	jz	.wait_output
	ret

; Fast A20 gate. This is the riskiest method, since it might not be supported on many BIOSes,
; and may do something weird. So it's the last one we'll try
a20_fast:
	in	al, 0x92
	test	al, 2		; Check if A20 already enabled
	jnz	.done
	or	al, 2		; Enable A20
	and	al, 0xfe	; Make sure bit 0 is clear, since it is used for fast reset
	out	0x92, al
.done:
	ret

; Test if the A20 line is enabled. Tries writing to the same memory accessed by wrapping around
; the 1MB mark. If wraps around, A20 is not enabled
; Writes to 0x500. This is [probably] going to be safe, since the only thing that could be here
; is the stack, and our stack is far, far away
; Returns - AX = 1 if enabled, AX = 0 if disabled.
test_a20:
	pushf
	push	ds
	push	es
	push	di
	push	si
	xor	ax, ax
	mov	es, ax		; ES = 0
	not	ax
	mov	ds, ax		; DS = 0xffff
	mov	di, 0x500
	mov	si, 0x510
	mov	byte [es:di], 0x00
	mov	byte [ds:si], 0xff
	cmp	byte [es:di], 0xff
	je	.no_a20
	mov	ax, 1
	jmp	.done
.no_a20:
	xor	ax, ax
.done:
	pop	si
	pop	di
	pop	es
	pop	ds
	popf
	ret


; Get VESA BIOS information, including manufacturer, supported modes, available video memory etc.
; Returns - AX = pointer to VESA BIOS info if valid, NULL otherwise
global real_vesa_get_bios_info
real_vesa_get_bios_info:
	push	es
	push	di
	; Set segment:offset for storing VESA BIOS info
	xor	ax, ax
	mov	es, ax
	mov	di, vbe_info
	; Read info
	mov	ax, 0x4f00
	int	0x10
	; Check return status
	cmp	ax, 0x004f
	jne	.no_vesa
	; Check signature
	cmp	byte [vbe_info.signature], 'V'
	jne	.no_vesa
	cmp	byte [vbe_info.signature + 1], 'E'
	jne	.no_vesa
	cmp	byte [vbe_info.signature + 2], 'S'
	jne	.no_vesa
	cmp	byte [vbe_info.signature + 3], 'A'
	jne	.no_vesa
	; Valid VESA. Set pointer
	mov	ax, vbe_info
	jmp	.done
.no_vesa:
	; NULL out pointer
	xor	ax, ax
.done:
	pop	di
	pop	es
	ret


; Get VESA mode information for given mode number
; Params - VESA mode number (word)
; Returns - Pointer to mode info struct if valid, NULL on failure
global real_vesa_get_mode_info
real_vesa_get_mode_info:
	; Store registers
	push	bp
	mov	bp, sp
	push	es
	push	di
	; Set buffer where we'll be storing mode info
	xor	ax, ax
	mov	es, ax
	mov	di, vbe_mode_info
	; Get mode number
	mov	cx, word [bp + 4]
	; Check if end mode
	cmp	cx, 0xffff
	je	.invalid
	; Get mode info
	mov	ax, 0x4f01
	int	0x10
	; Check return
	cmp	ax, 0x004f
	jne	.invalid
	; Return pointer
	mov	ax, vbe_mode_info
	jmp	.done
.invalid:
	; Return NULL
	xor	ax, ax
.done:
	; Restore registers and return
	pop	di
	pop	es
	pop	bp
	ret


; Set VESA video mode.
; Params  - VESA mode number (word)
; Returns - 1 on success, 0 on failure
global real_vesa_set_mode
real_vesa_set_mode:
	; Store BX
	push	bp
	mov	bp, sp
	push	bx
	; Get mode number
	mov	bx, word [bp + 4]
	cmp	bx, 0xffff
	je	.invalid
	; Set mode
	and	bx, 0x7fff  ; Zero-out bit 15
	or	bx, 0x4000  ; Set bit 14, enable linear framebuffer
	mov	ax, 0x4f02
	int	0x10
	; Check return
	cmp	ax, 0x004f
	jne	.invalid
	; Return success
	mov	ax, 1
	jmp	.done
.invalid:
	xor	ax, ax
.done:
	; Restore registers and return
	pop	bx
	pop	bp
	ret


;; ---------------- 32-BIT PROTECTED MODE -----------------

[BITS 32]

; Entry point of 32-bit code
protected_mode_start:
	; Set segment registers
	mov	ax, 0x10
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax

	; Enable NMI
	in	al, 0x70
	and	al, 0x7e
	out	0x70, al

	; Jump to C code
	mov	sp, 0x7c00
	sub	sp, 12
	mov	eax, MEM_MAP_BASE
	push	eax
	extern	main
	call	main


; Halt the processor
global halt_32
halt_32:
	cli
	hlt
	jmp	halt_32



section .data

; Structure to store VESA BIOS information
align 4
vbe_info:
	.signature:        db "VBE2"            ; Must be "VESA" to indicate valid VBE support
	.version_min       db 0                 ; VBE minor version
	.version_maj       db 0                 ; VBE major version
	.oem_off:          dw 0                 ; Offset to OEM
	.oem_seg:          dw 0                 ; Segment for OEM
	.capabilities:     dd 0                 ; Bitfield that describes card capabilities
	.video_modes_off:  dw 0                 ; Offset for supported video modes
	.video_modes_seg:  dw 0                 ; Segment for supported video modes
	.video_memory:     dw 0                 ; Amount of video memory in 64KB blocks
	.software_rev:     dw 0                 ; Software revision
	.vendor_off:       dw 0                 ; Offset to card vendor string
	.vendor_seg:       dw 0                 ; Segment for card vendor string
	.product_name_off: dw 0                 ; Offset for product name
	.product_name_seg: dw 0                 ; Segment for product name
	.product_rev_off:  dw 0                 ; Offset for product revision
	.product_rev_seg:  dw 0                 ; Segment for product revision
	.rsvd:             times 222 db  0      ; Reserved
	.oem_data:         times 256 db  0      ; OEM BIOSes store their strings here


; Structure of VESA mode information
align 4
vbe_mode_info:
	.attributes              : dw 0
	.window_a                : db 0
	.window_b                : db 0
	.granularity             : dw 0
	.window_size             : dw 0
	.segment_a               : dw 0
	.segment_b               : dw 0
	.win_func_ptr            : dd 0
	.pitch                   : dw 0
	.width                   : dw 0
	.height                  : dw 0
	.w_char                  : db 0
	.y_char                  : db 0
	.planes                  : db 0
	.bpp                     : db 0
	.banks                   : db 0
	.model                   : db 0
	.bank_size               : db 0
	.image_pages             : db 0
	.reserved0               : db 0
	.red_mask_size           : db 0
	.red_mask_pos            : db 0
	.green_mask_size         : db 0
	.green_mask_pos          : db 0
	.blue_mask_size          : db 0
	.blue_mask_pos           : db 0
	.rsvd_mask_size          : db 0
	.rsvd_mask_pos           : db 0
	.direct_color_attributes : db 0
	.framebuffer             : dd 0
	.off_screen_mem_off      : dd 0
	.off_screen_mem_size     : dw 0
	.reserved1               : times 206 db 0


; 32-bit GDT
align 8
global gdt32
gdt32:
.null:	equ 0
	dd	0, 0
; 32-bit code segment
.code32:	equ ($ - gdt32)
	dw	0xffff		; Limit_low
	dw	0		; Base_low
	db	0		; Base mid
	db	0x9a		; Access byte (Ring 0 code segment)
	db	0xcf		; Flags | Limit_high
	db	0		; Base_high
; 32-bit data segment
.data32:	equ ($ - gdt32)
	dw	0xffff		; Limit_low
	dw	0		; Base_low
	db	0		; Base mid
	db	0x92		; Access byte (Ring 0 data segment)
	db	0xcf		; Flags | Limit_high
	db	0		; Base_high
; 16-bit code segment
.code16:	equ ($ - gdt32)
	dw	0xffff		; Limit_low
	dw	0		; Base_low
	db	0		; Base mid
	db	0x9a		; Access byte (Ring 0 code segment)
	db	0x00		; Flags | Limit_high
	db	0		; Base_high
; 16-bit data segment
.data16:	equ ($ - gdt32)
	dw	0xffff		; Limit_low
	dw	0		; Base_low
	db	0		; Base mid
	db	0x92		; Access byte (Ring 0 data segment)
	db	0x00		; Flags | Limit_high
	db	0		; Base_high


; GDT register
global gdtr32
gdtr32:
.size:	dw ($ - gdt32) - 1
.off:	dd gdt32



section .rodata

; Strings

msg_mem_map_err:         db "[X] Mem map", 0x0A, 0x0D, 0x00
msg_mem_map_ok:          db "[+] Mem map", 0x0A, 0x0D, 0x00
msg_no_a20:              db "[X] A20 gate", 0x0A, 0x0D, 0x00

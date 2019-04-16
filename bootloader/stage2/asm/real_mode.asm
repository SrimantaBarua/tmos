; (C) 2019 Srimanta Barua
;
; BIOS interrupts are useful. Atleast for a bootloader, I guess it's okay to use them.
; This file contains an entry-point which switches to 16-bit real mode, and then runs a
; function which was passed to it as a parameter.


extern halt_32
extern vlog
extern log


LOG_INFO:     equ 0
LOG_WARN:     equ 1
LOG_ERR:      equ 2
LOG_SUCCESS:  equ 3


section .text



[BITS 16]


; -------- Disk I/O ----------------

; Check if BIOS interrupt 0x13 extensions are supported
; These extensions allow us to read disk sectors using LBA, instead of CHS
; This interrupt requires that -
;     AH = 0x41
;     BX = 0x55AA
;     DL = <drive number>
; If supported -
;     Carry flag is clear
;     BX = 0xAA55
;     CX & 0x01 == 1 (Device access using packet structure supported)
; Params   - drive_number (byte)
; Returns  - AX = 1 if supported, 0 if not supported
global real_disk_check_int13_ext:
real_disk_check_int13_ext:
	; Store registers
	push	bp
	mov	bp, sp
	push	bx
	push	dx
	; Check for extension
	mov	dl, byte [bp + 4]   ; Drive number
	mov	bx, 0x55aa
	mov	ah, 0x41
	int	0x13
	jc	.no_ext
.ext:
	; Check return signature
	cmp	bx, 0xAA55
	jne	.no_ext
	; Check for DAP support
	test	cx, 0x01
	jz	.no_ext
	mov	ax, 1
	jmp	.done
.no_ext:
	xor	ax, ax
.done:
	; Restore registers and return
	pop	dx
	pop	bx
	pop	bp
	ret


; Read sectors using int 0x13 extensions
; This uses a Disk Address Packet structure and passes a pointer to it to the interrupt, in DS:SI
; The Disk Address Packet structure looks like -
;
; struct disk_address_packet {
;     uint8_t size;          // Size of DAP (16 bytes)
;     uint8_t _rsvd;         // Reserved (should be 0)
;     uint16_t num_sectors;  // Number of sectors to read. Some Phoenix BIOSes limit to  max 127
;     uint16_t off;          // Offset part of pointer to memory buffer
;     uint16_t seg;          // Segment part of pointer to memory buffer
;     uint64_t start_sector; // Absolute number of start sector (first sector of disk is 0)
; };
;
; Params   - drive_num (byte), off (word), seg (word) - making up the seg:off pointer to the DAP
; Returns  - AX = 1 on success, 0 on failure
global real_disk_read_sectors:
real_disk_read_sectors:
	; Store registers
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	dx
	; Load from arguments
	mov	dl, byte [bp + 4]     ; Drive number
	mov	si, word [bp + 6]     ; Offset pointing to DAP
	mov	ax, word [bp + 8]     ; Segment pointing to DAP
	mov	ds,  ax
	; Read sectors
	mov	ah, 0x42
	int	0x13
	jc	.err
.ok:
	mov	ax, 1
	jmp	.done
.err:
	xor	ax, ax
.done:
	; Restore registers and return
	pop	dx
	pop	si
	pop	ds
	pop	bp
	ret


; Read extended drive parameters
; This return data in a result buffer, which looks like -
;
; struct result_buffer {
;     uint16_t size;                // Size of result buffer (30 bytes)
;     uint16_t flags;               // Information flags
;     uint32_t cylinders;           // Physical number of cylinders
;     uint32_t heads;               // Physical number of heads
;     uint32_t sectors_per_track;   // Physical number of sectors per track
;     uint64_t sectors;             // Absolute number of sectors
;     uint16_t bytes_per_sector;    // Bytes per sector
;     uint32_t edd_ptr;             // Optional pointer to enhanced disk drive params
; }
; Params   - drive_num (byte), off (word), seg (word) - making up the seg:off pointer to buffer
; Returns  - AX = 1 on success, 0 on failure
global real_disk_read_ext_params
real_disk_read_ext_params:
	; Store registers
	push	bp
	mov	bp, sp
	push	ds
	push	si
	push	dx
	; Load from arguments
	mov	dl, byte [bp + 4]     ; Drive number
	mov	si, word [bp + 6]     ; Offset pointing to buffer
	mov	ax, word [bp + 8]     ; Segment pointing to buffer
	mov	ds,  ax
	; Read sectors
	mov	ah, 0x42
	int	0x13
	jc	.err
.ok:
	mov	ax, 1
	jmp	.done
.err:
	xor	ax, ax
.done:
	; Restore registers and return
	pop	dx
	pop	si
	pop	ds
	pop	bp
	ret



; -------- VESA ----------------

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


; -------- Switch code ----------------


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

; Temporary store for CR0
save_cr0:  dd 0


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


section .rodata

; Strings
msg_ptr_too_high: db "Pointer too high: 0x%x", 0x0a, 0x00
msg_ptr_null:     db "Pointer is NULL", 0x0a, 0x00

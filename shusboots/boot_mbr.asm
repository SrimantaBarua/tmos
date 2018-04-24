; boot_mbr.asm
; Entry point of the MBR-based bootloader
; (C) 2018 Srimanta Barua
;
; The BIOS loads 512 bytes of boot code from the beginning of the disk, at 0x7c00. This code has
; to then load up the rest of the system. We start by loading the rest of the bootloader at 0x7e00,
; which then loads the rest of the system.
;
; The MBR layout for our bootloader is similar to a classical MBR -
; 0x000 .. 0x1BE  ----  Bootstrap code
; 0x1BE .. 0x1FE  ----  Partition Entries [4] (each 16 bytes)
; 0x1FE .. 0x200  ----  MBR signature (0xAA55)
;
; Partition entries look like this -
;  0 ..  1  ----  Status (0x80 for bootable)
;  1 ..  4  ----  CHS address of first absolute sector in partition
;  4 ..  5  ----  Partition type (0x7F for experimental development)
;  5 ..  8  ----  CHS address of last absolute sector in partition
;  8 .. 12  ----  LBA of first absolute sector in partition
; 12 .. 16  ----  Number of sectors in partition
;
; CHS addresses look like this -
; Byte 0  ----  Head
; Byte 1  ----  Lower 6 bits = sector | Upper 2 bits = high bits of cylinder
; Byte 2  ----  Bits 7-0 of cylinder
;
; LBA is a linear addressing scheme. Conversion between LBA and CHS is done as follows -
;
; LBA = (C x HPC + H) x SPT + (S - 1)
;     where,
;         C, H, S are cylinder, head and sector number
;         HPC is maximum number of heads per cylinder
;         SPT is maximum number of sectors per track
;
; C = LBA / (HPC x SPT)
; H = (LBA / SPT) % HPC
; S = (LBA % SPT) + 1


;; ---------------- STAGE 1 -----------------

[ORG 0x7c00]
[BITS 16]


; Some BIOSes load us at 0x07c0:0x0000 while others load us at 0x0000:0x7c00. Normalize to
; 0x0000:0x7c00 with a long jump
start:
	cli
	jmp	0:main


; Print a '\0' - terminated string using BIOS interrupt 0x10
; This interrupt requires that -
;     AH = 0x0E
;     AL = <ascii char to print>
; Params -  Pointer to string in DS:SI
; Clobbers - SI, Flags
print_16:
	push	ax
.loop:
	lodsb
	or	al, al
	jz	.done
	mov	ah, 0x0e
	int	0x10
	jmp	.loop
.done:
	pop	ax
	ret


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
; Returns - Ax = 1 if supported, 0 if not supported
; Clobbers - AX, Flags
check_int13_ext:
	push	bx
	push	dx
	mov	dl, byte [drive_number]
	mov	bx, 0x55aa
	mov	ah, 0x41
	int	0x13
	jc	.no_ext
.ext:
	cmp	bx, 0xAA55
	jne	.no_ext
	test	cx, 0x01
	jz	.no_ext
	mov	ax, 1
	jmp	.done
.no_ext:
	xor	ax, ax
.done:
	pop	dx
	pop	bx
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
; Params -
;     CX = number of sectors to read
;     AX = sector number. We won't be reading higher than this for some time
;     ES:DI = seg:off pointer to memory buffer
; Returns -
;     AX = 1 on success, 0 on failure
read_sectors_int13_ext:
	push	ds
	push	si
	push	dx
	mov	byte [.dap_size], 16
	mov	word [.dap_num_sectors], cx
	mov	word [.dap_ptr], di
	mov	word [.dap_ptr + 2], es
	mov	word [.dap_start_sector], ax
	xor	ax, ax
	mov	ds, ax
	mov	si, .dap
	mov	ah, 0x42
	mov	dl, byte [drive_number]
	int	0x13
	jc	.err
.ok:
	mov	ax, 1
	jmp	.done
.err:
	xor	ax, ax
.done:
	pop	dx
	pop	si
	pop	ds
	ret

align 4
.dap:
.dap_size:
	db 0x10
.dap_unused:
	db 0
.dap_num_sectors:
	dw 0
.dap_ptr:
	times 2 dw 0
.dap_start_sector:
	times 4 dw 0


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
	; Respore registers
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



; Real starting point of the MBR code
; - Set segments to 0
; - Check for int 0x13 extensions
; - Load rest of the bootloader code
main:
	; Set segments
	xor	ax, ax
	mov	ds, ax
	mov	es, ax

	; Set up stack
	mov	ss, ax
	mov	sp, 0x7c00

	; Store drive number
	mov	byte [drive_number], dl

	; Check for int 0x13 extensions
	call	check_int13_ext
	mov	byte [int_13_support], al
	or	ax, ax
	jnz	.int13_ext_present
.no_int13_ext:
	mov	si, msg_int13_ext_no
	call	print_16
	jmp	halt
.int13_ext_present:
	mov	si, msg_int13_ext_yes
	call	print_16

	; Load the next sector, which contains stage 1.5
	mov	ax, 0x07e0
	mov	es, ax
	xor	di, di
	mov	ax, 1
	mov	cx, 1
	call	read_sectors_int13_ext
	or	ax, ax
	jz	.no_read_sectors
.read_sectors:
	; Jump to starting of the code for stage 1.5
	jmp	stage_1_5_start
.no_read_sectors:
	; Print message
	xor	ax, ax
	mov	ds, ax
	mov	si, msg_read_sectors_no
	call	print_16

	jmp	halt


; Halt infinitely
halt:
	cli
	hlt
	jmp	halt


; Data
msg_int13_ext_yes: db "[+] int 13h ext", 0x0A, 0x0D, 0x00
msg_int13_ext_no:  db "[X] int 13h ext", 0x0A, 0x0D, 0x00
msg_read_sectors_no: db "[X] read_sectors failed", 0x0A, 0x0D, 0x00

drive_number:   db 0
int_13_support: db 0



; Padding till we get to the partition tables
times 0x1BE - ($ - $$) db 0

; Partition tables
part_0:
	.status:	db 0
	.chs_first:	times 3 db 0
	.type:		db 0
	.chs_last:	times 3 db 0
	.lba_first:	times 4 db 0
	.num_sectors:	times 4 db 0
part_1:
	.status:	db 0
	.chs_first:	times 3 db 0
	.type:		db 0
	.chs_last:	times 3 db 0
	.lba_first:	times 4 db 0
	.num_sectors:	times 4 db 0
part_2:
	.status:	db 0
	.chs_first:	times 3 db 0
	.type:		db 0
	.chs_last:	times 3 db 0
	.lba_first:	times 4 db 0
	.num_sectors:	times 4 db 0
part_3:
	.status:	db 0
	.chs_first:	times 3 db 0
	.type:		db 0
	.chs_last:	times 3 db 0
	.lba_first:	times 4 db 0
	.num_sectors:	times 4 db 0

; Padding till the end of the file
times 510 - ($ - $$) db 0

; MBR signature
dw 0xAA55




;; ---------------- STAGE 1.5 -----------------

; Start of the stage 1.5 bootloader. This loads C code, makes the switch to 32-bit protected mode,
; and jumps to the C code. It also does the following things -
; - Reads the BIOS memory map
; - Gives user a menu to select display mode. (As of now, this mode will stick till next reset)
stage_1_5_start:
	; Read BIOS memory map to 0x1000:0x0000 (0x10000)
	mov	ax, 0x1000
	mov	es, ax
	xor	di, di
	call	int_15_e820_read_mmap
	or	ax, ax
	jnz	.mem_map_ok
.mem_map_err:
	mov	ds, ax
	mov	si, msg_mem_map_err
	call	print_16
	jmp	halt
.mem_map_ok:
	xor	ax, ax
	mov	ds, ax
	mov	si, msg_mem_map_ok
	call	print_16
	; Halt
	jmp	halt


msg_mem_map_err: db "[X] Mem map", 0x0A, 0x0D, 0x00
msg_mem_map_ok:  db "[+] Mem map", 0x0A, 0x0D, 0x00

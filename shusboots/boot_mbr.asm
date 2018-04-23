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
	jz	.no_int13_ext
.int13_ext:
	mov	si, msg_int13_ext_yes
	jmp	.int13_ext_done
.no_int13_ext:
	mov	si, msg_int13_ext_no
.int13_ext_done:
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

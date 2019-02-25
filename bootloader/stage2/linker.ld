ENTRY(stage2_start)

SECTIONS {
	. = 0x7e00;

	.stage2_start_section : AT(0x7e00) {
		KEEP(*(.stage2_start_section*));
	}

	.text : {
		*(.text*);
		*(.init*);
		*(.fini*);
	}

	.rodata : {
		*(.rodata*);
	}

	.data : {
		*(.data*);
	}

	.bss : {
		*(.bss*);
		*(COMMON);
	}

	/DISCARD/ : {
		*(.comment*);
	}
}

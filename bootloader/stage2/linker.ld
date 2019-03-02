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
		__bss_start_addr__ = .;
		*(.bss*);
		*(COMMON);
		__bss_end_addr__ = .;
	}

	/DISCARD/ : {
		*(.comment*);
	}
}

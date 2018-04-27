ENTRY(stage2_start)

SECTIONS {
	. = 0x8200;

	.stage2_start_section : AT(0x8200) {
		KEEP(*(.stage2_start_section*));
	}

	.text : {
		*(.text*);
	}

	.data : {
		*(.bss*);
		*(COMMON);
		*(.rodata*);
		*(.data*);
	}

	/DISCARD/ : {
	}
}

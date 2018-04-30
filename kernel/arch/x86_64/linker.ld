ENTRY(_start)

SECTIONS {
	. = 1M;

	.bootstrap : {
		KEEP(*(.multiboot*))
		*(.bootstrap*)
		. = ALIGN(4K);
	}
}


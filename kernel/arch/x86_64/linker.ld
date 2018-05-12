ENTRY(_start)

SECTIONS {
	__kernel_vbase__ = 0xffffffff80000000;

	. = 1M;

	.bootstrap : {
		KEEP(*(.multiboot*))
		*(.bootstrap*)
		. = ALIGN(4K);
	}

	__kernel_phys_start__ = .;
	. = __kernel_phys_start__ + __kernel_vbase__;
	__kernel_virt_start__ = .;

	.text : AT(ADDR(.text) - __kernel_vbase__) {
		__text_start__ = .;
		*(.text*)
		*(.init*)
		*(.fini*)
		. = ALIGN(4K);
		__text_end__ = .;
	}
	.rodata : AT(ADDR(.rodata) - __kernel_vbase__) {
		__rodata_start__ = .;
		*(.rodata*)
		. = ALIGN(4K);
		__rodata_end__ = .;
	}

	.data : AT(ADDR(.data) - __kernel_vbase__) {
		__data_start__ = .;
		*(.data*);
		. = ALIGN(4K);
		__data_end__ = .;
	}

	.bss : AT(ADDR(.bss) - __kernel_vbase__) {
		__bss_start__ = .;
		*(.bss*);
		. = ALIGN(4K);
		__bss_end__ = .;

		__kernel_phys_end__ = . - __kernel_vbase__;
		__kernel_virt_end__ = .;
	}

	/DISCARD/ : {
		*(.comment*)
	}
}


OUTPUT_FORMAT("binary")
OUTPUT_ARCH(i386)
ENTRY(start)

phys = 0x00100000;

SECTIONS {
	.text phys : AT(phys) {
		code = .;
		*(.text)
        src/init.o(.text)
		*(.rodata)
		. = ALIGN(4096);
	}

	.data : AT(phys + (data - code)) {
		data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.bss : AT(phys + (bss - code)) {
		bss = .;
		*(.bss)
		. = ALIGN(4096);
        bss_end = .;
	}

	end =.;
}

OUTPUT_FORMAT("binary")
ENTRY(start)

phys = 0x00100000;

SECTIONS {
	.text phys : AT(phys) {
		code = .;
		*(.text)
		*(.rodata)
		. = ALIGN(4096);
	}

	.data : AT(phys + (data - code)) {
		data = .;
		*(.data)
		. = ALIGN(4096);
	}

	.bss : AT(phys + (bss - code)) {
        bss_start = ABSOLUTE(.);
		bss = .;
		*(.bss)
		. = ALIGN(4096);
        bss_size = ABSOLUTE(.) - bss_start;
	}

	end =.;
}

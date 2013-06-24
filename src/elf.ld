OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(start)

phys = 0x0100000;

SECTIONS
{
    . = phys;

    .text ALIGN(4096) : AT( ADDR(.text) )
    {
        code = .;
        src/init.o(.text)
        *(.text)
        *(.rodata)
        code_end = .;
    }

    .data ALIGN(4096) : AT( ADDR(.data) )
    {
        data = .;
        *(.data)
        data_end = .;
    }

    .bss ALIGN(4096) : AT( ADDR(.bss) )
    {
        bss = .;
        *(.bss)
        bss_end = .;
    }

    end = .;
}

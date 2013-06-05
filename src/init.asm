	[bits 32]

	global start

start:
	; Initialize stack
	mov esp, stack

	; Run initialization code
	jmp init

	align 4
mboot:
	; Multiboot macros
	MULTIBOOT_PAGE_ALIGN equ 1 << 0
	MULTIBOOT_MEMORY_INFO equ 1 << 1
	MULTIBOOT_AOUT_KLUDGE equ 1 << 16
	MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
	MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_AOUT_KLUDGE
	MULTIBOOT_CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

	extern code, bss, end

	; GRUB multiboot signature
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

	; AOUT kludge (filled in by linker)
	dd mboot
	dd code
	dd bss
	dd end
	dd start

; Do everything needed to run the main game
init:
	; Just clear screen in text mode for now
	mov eax, 0xB8000
zero:
	mov dword [eax], 0
	add eax, 4
	cmp eax, 0xB8FA0
	jne zero

	; Stop
	jmp $

; Static allocations
	section .bss
	stack resb 8192
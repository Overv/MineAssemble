[bits 32]

global main

extern set_timer_frequency, set_irq_handler, enable_irq
extern irq1_end

section .text

	main:
		; Simply flash screen blue/green on key press

		push dword irq1
		push 1
		call set_irq_handler
		call enable_irq
		add esp, 8

    loop:
        mov ebx, [color]

        mov edi, 0xa0000
        mov ah, bl
        mov al, bl
        mov ecx, 0x20000
        rep stosw

        jmp loop

		ret

	irq1:
    kbwait:
        in al, 0x64
        and al, 1
        test al, al
        jz kbwait

        in al, 0x60
        cmp al, 0x1c
        jnz skip

        mov eax, 3
        sub eax, [color]
        mov [color], eax
skip:
		jmp irq1_end

section .data
        color dd 2

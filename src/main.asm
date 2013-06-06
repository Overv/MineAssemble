[bits 32]

global main

extern set_timer_frequency, set_irq_handler, enable_irq
extern irq0_end

section .text

	main:
		; Simply flash screen blue/green

		push 1000
		call set_timer_frequency
		add esp, 4

		push dword irq0
		push 0
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

	irq0:
        inc dword [cycle]
        cmp dword [cycle], 250
        jnz skip
        mov dword [cycle], 0
        mov eax, 3
        sub eax, [color]
        mov [color], eax
skip:
		jmp irq0_end

section .data
        color dd 2
        cycle dd 0

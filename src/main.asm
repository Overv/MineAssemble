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

        mov ecx, 2

		jmp $

		ret

	irq0:
        mov ebx, 3
        sub ebx, ecx
        mov ecx, ebx

        push ecx

		mov edi, 0xa0000
		mov ah, bl
        mov al, bl
		mov ecx, 0x20000
		rep stosw

        pop ecx

		jmp irq0_end

[bits 32]

global init_idt, init_pic

global set_timer_frequency

global set_irq_handler, clear_irq_handler, enable_irq
global irq0_end, irq1_end, irq2_end, irq3_end, irq4_end, irq5_end, irq6_end, irq7_end
global irq8_end, irq9_end, irqA_end, irqB_end, irqC_end, irqD_end, irqE_end, irqF_end

section .bss

	; IDT memory
	idt resb 0x800

section .text

	; Initialize Interrupt Descriptor Table (IDT)
	idt_ptr:
		dw 0x800 - 1
		dd idt

	init_idt:
		lidt [idt_ptr]
		ret

	; Initialize Programmable Interrupt Controllers (PIC)
	init_pic:
		mov al, 0x11 ; Init command
		out 0x20, al ; PIC1 command port
		out 0xA0, al ; PIC2 command port

		mov al, 0x20 ; Interrupt offset
		out 0x21, al ; PIC1 data port
		mov al, 0x28 ; Interrupt offset
		out 0xA1, al ; PIC2 data port

		mov al, 0x04 ; IR4 is connected to slave (PIC2)
		out 0x21, al ; PIC1 data port
		mov al, 0x02 ; Slave ID 2
		out 0xA1, al ; PIC2 data port

		mov al, 0x01 ; 8086/88 mode
		out 0x21, al ; PIC1 data port
		mov al, 0x01 ; 8086/88 mode
		out 0xA1, al ; PIC2 data port

		mov al, 0xFF ; Disable all IRQs
		out 0x21, al ; PIC1 data port
		mov al, 0xFF ; Disable all IRQs
		out 0xA1, al ; PIC1 data port

		push dword [spurious_interrupt_handler]
		push 7
		call set_irq_handler
		add esp, 4
		push 15
		call set_irq_handler
		add esp, 8

		ret

	; Set IRQ0 timer frequency
    ; Must be at least 16 Hz
	set_timer_frequency:
		mov edx, 0
		mov eax, 0x123456
		div dword [esp + 4]
		push eax
		call set_timer_rate
		add esp, 4
		ret

	; Set IRQ0 timer rate
	set_timer_rate:
		mov al, 0x34 ; Channel 0 (rate generator mode)
		out 0x43, al ; PIT mode/command port
		mov eax, [esp + 4]
		out 0x40, al ; PIT channel 0 data port
		mov al, ah
		out 0x40, al
		ret

	spurious_interrupt_handler:
		iret

	; Create or update IDT entry
	set_interrupt_handler:
		cli

		mov ecx, [esp + 4] ; Interrupt index
		mov ax,  [esp + 8]
		mov [0 + idt + ecx * 8], ax ; Low word of handler address
		mov ax, [esp + 10]
		mov [6 + idt + ecx * 8], ax ; High word of handler address
		mov ax, cs
		mov [2 + idt + ecx * 8], ax ; Code segment selector
		mov word [4 + idt + ecx * 8], 0x8E00 ; Attributes (0x8E00 = 32-bit interrupt gate)

		sti

		ret

	; Remove IDT entry
	clear_interrupt_handler:
		mov ecx, [esp + 4] ; Interrupt index
		mov word [4 + idt + ecx * 8], 0 ; Attributes (0 = Not used)
		ret

	; Create or update IDT entry by IRQ index
	set_irq_handler:
		mov ecx, [esp + 4] ; Index
		mov esi, [esp + 8] ; Handler address
		add ecx, 0x20 ; Interrupt offset
		push esi
		push ecx
		call set_interrupt_handler
		add esp, 8
		ret

	; Remove IDT entry for IRQ index
	clear_irq_handler:
		mov ecx, [esp + 4]
		add ecx, 0x20
		push ecx
		call clear_interrupt_handler
		add esp, 4
		ret

	; Enable IRQ
	enable_irq:
		mov dx, 0x21
		mov al, [esp + 4]
		shl al, 4
		and al, 0x80
		add dl, al
		mov cl, [esp + 4]
		and cl, 111b
		mov bl, 1
		shl bl, cl
		in al, dx
		not bl
		and al, bl
		out dx, al
		ret

	; IRQ finishing
	irq0_end:
	irq1_end:
	irq2_end:
	irq3_end:
	irq4_end:
	irq5_end:
	irq6_end:
	irq7_end:
		; IRQs 0 to 7 are controlled by PIC1
		push eax
		mov al, 0x20
		out 0x20, al
		pop eax
		iret

	irq8_end:
	irq9_end:
	irqA_end:
	irqB_end:
	irqC_end:
	irqD_end:
	irqE_end:
	irqF_end:
		; IRQs 8 to F are controlled by PIC2
		push eax
		mov al, 0x20
		out 0xA0, al
		out 0x20, al
		pop eax,
		iret

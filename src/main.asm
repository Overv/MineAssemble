[bits 32]

global main

section .text

	main:
		; Just clear screen in text mode for now
		mov eax, 0xB8000
	zero:
		mov dword [eax], 0
		add eax, 4
		cmp eax, 0xB8FA0
		jne zero

		ret
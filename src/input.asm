;
; This file contains all functions related to input handling
;

[bits 32]

%include "constants.asm"

global handle_input

extern handle_key

section .text

    ; Handle input of all key events
    handle_input:
        push dword KEY_UP
        call handle_key

        mov dword [esp], KEY_DOWN
        call handle_key

        mov dword [esp], KEY_LEFT
        call handle_key

        mov dword [esp], KEY_RIGHT
        call handle_key

        mov dword [esp], KEY_SPACE
        call handle_key

        mov dword [esp], KEY_Q
        call handle_key

        mov dword [esp], KEY_E
        call handle_key

        mov dword [esp], KEY_ESC
        call handle_key

        add esp, 4

        ret
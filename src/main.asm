;
; This file contains the game initialization and main loop code
;

[bits 32]

global main

extern init_world, handle_input, update, draw_frame
extern time

section .data
    msPerSecond dd 1000.0

    ; Last update time in milliseconds
    lastUpdate dd 0

section .text

    main:
        push ebp
        mov ebp, esp

        ; Make room for deltaTime variable
        sub esp, 4

        ; Initialize world array
        call init_world

        ; Main loop
    .main_loop:
        ; Read input buffer and update motion state
        call handle_input

        ; Calculate delta time
        ; (time - lastUpdate) / 1000
        fild dword [time]
        fild dword [lastUpdate]
        fsub
        fld dword [msPerSecond]
        fdiv
        fstp dword [ebp - 4]

        ; Update world state
        push dword [ebp - 4]
        call update
        add esp, 4

        ; Save last update time
        mov eax, [time]
        mov [lastUpdate], eax

        ; Draw next frame
        call draw_frame

        jmp .main_loop
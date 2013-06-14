;
; This file contains all functions related to input handling
;

[bits 32]

%include "constants.asm"

global handle_input, handle_collision

extern handle_key, raytrace

extern colTolerance, zero

section .text

    ; void handle_input()
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

    ; void handle_collision(vec3 pos, vec3* velocity)
    ; Adjust velocity to prevent any collisions
    handle_collision:
        push ebp
        mov ebp, esp

        ; Allocate space for hit info struct
        sub esp, 32

        ; Trace ray with velocity as direction to check for collision
        mov eax, ebp
        sub eax, 32
        push eax ; Pointer to hit info struct

        mov eax, [ebp + 20] ; velocity pointer
        push dword [eax + 8] ; Copy of velocity vec3 (in reverse because stack grows downwards)
        push dword [eax + 4]
        push dword [eax + 0]

        push dword [ebp + 16] ; Copy of pos vec3
        push dword [ebp + 12]
        push dword [ebp + 8]

        call raytrace

        add esp, 28

        ; Check for hit
        cmp byte [ebp - 32], 1
        jne .finish

        ; Check if distance is < 0.1
        fld dword [colTolerance]
        fld dword [ebp - 4]
        fcomip
        fstp dword [ebp + 8] ; Discard colTolerance still on stack by writing over now unused pos
        jae .finish

        ; Correct velocity to create sliding motion over surface

        mov ecx, [zero] ; floating point zero
        mov eax, [ebp + 20] ; velocity pointer

        cmp dword [ebp - 16], 0 ; nx != 0 -> negate x velocity
        je .nx_finish
        mov dword [eax + 0], ecx
    .nx_finish:

        cmp dword [ebp - 12], 0 ; ny != 0 -> negate y velocity
        je .ny_finish
        mov dword [eax + 4], ecx
    .ny_finish:

        cmp dword [ebp - 8], 0 ; nz != 0 -> negate z velocity
        je .finish
        mov dword [eax + 8], ecx

    .finish:
        mov esp, ebp
        pop ebp

        ret
;
; This file contains all code related to the player state
;

[bits 32]

global set_pos, set_view

global playerHeight

global playerPos
global pitch, pitchS, pitchC
global yaw, yawS, yawC

extern half_pi, nhalf_pi

section .data

        ; Player height
        playerHeight dd 1.8

section .bss
        ; Player position (3 floats)
        playerPos resd 3

        ; Player orientation (pitch and yaw + cached sine/cosine)
        pitch resd 1
        pitchS resd 1
        pitchC resd 1

        yaw resd 1
        yawS resd 1
        yawC resd 1

section .text

    ; Set player position (x, y, z)
    set_pos:
        mov eax, [esp + 4]
        mov [playerPos + 0], eax  ; x -> playerPos.x
        mov eax, [esp + 8]
        mov [playerPos + 4], eax  ; y -> playerPos.y
        mov eax, [esp + 12]
        mov [playerPos + 8], eax  ; z -> playerPos.z

        ret

    ; Set player view pitch and yaw
    set_view:
        mov eax, [esp + 4]
        mov [pitch], eax ; p param -> pitch

        ; Limit pitch between -pi/2 and pi/2
        fld dword [pitch]
        fld dword [half_pi]
        fcomip
        ja .pitch_not_too_big ; jump if pitch < 1.57
        fld dword [half_pi]
        fstp dword [pitch]
        jmp .pitch_ok
    .pitch_not_too_big:
        fld dword [nhalf_pi]
        fcomip
        jbe .pitch_ok ; jump if pitch > -1.57
        fld dword [nhalf_pi]
        fstp dword [pitch]
    .pitch_ok
        fstp dword [esp - 4] ; Discard original pitch still on stack

        ; Calculate pitch sin/cos
        fld dword [pitch]
        fsincos
        fstp dword [pitchC]
        fstp dword [pitchS]

        ; Calculate yaw sin/cos
        fld dword [yaw]
        fsincos
        fstp dword [yawC]
        fstp dword [yawS]

        mov eax, [esp + 8]
        mov [yaw], eax ; y param -> yaw

        ret
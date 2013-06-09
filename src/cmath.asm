;
; This file contains an implementation of part of the C math library
;

[bits 32]

global sinf, cosf, tanf, atanf

section .text
    sinf:
        fld dword [esp+4]
        fsin
        ret

    cosf:
        fld dword [esp+4]
        fcos
        ret

    tanf:
        fld dword [esp+4]
        fptan
        fstp dword [temp]
        ret

    atanf:
        fld dword [esp+4]
        fld1
        fpatan
        ret

section .data
    temp dd 0.0
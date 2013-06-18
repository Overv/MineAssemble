;
; This file contains an implementation of part of the C math library
;

[bits 32]

global sinf, cosf, absf

section .text
    sinf:
        fld dword [esp + 4]
        fsin
        ret

    cosf:
        fld dword [esp + 4]
        fcos
        ret

    absf:
        fld dword [esp + 4]
        fabs
        ret

section .data
    temp dd 0.0
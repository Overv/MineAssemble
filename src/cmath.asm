;
; This file contains an implementation of part of the C math library
;

[bits 32]

global sinf, cosf

section .text
    sinf:
        fld dword [esp+4]
        fsin
        ret

    cosf:
        fld dword [esp+4]
        fcos
        ret

section .data
    temp dd 0.0
;
; This file contains all functions related to rendering the world
;

[bits 32]

extern hFov
extern width, height, aspect, naspect, half
extern yawC, yawS, pitchC, pitchS

section .data

        doubleCircle dd 720.0

section .text

    global rayDir

rayDir:
    push ebp
    mov ebp, esp

    ; Allocate local variables (vFov, fov, clipX, clipY, length)
    sub esp, 20

    ; Load struct address
    mov eax, dword [ebp + 8]

    ; Calculate vertical fov and fov constant from horizontal fov
    fld1
    fadd st0
    fld dword [hFov]
    fld dword [doubleCircle]
    fdiv
    fldpi
    fmul
    fptan
    fstp dword [eax]
    fild dword [width]
    fmul
    fild dword [height]
    fdiv
    fld1
    fpatan
    fmul
    fst dword [ebp - 4]

    fld dword [half]
    fmul
    fptan
    fstp dword [eax]
    fstp dword [ebp - 8]

    ; clip X
    fild dword [ebp + 12]
    fild dword [width]
    fld dword [half]
    fmul
    fdiv
    fld1
    fsub
    fstp dword [ebp - 12]

    ; clip Y
    fld1
    fild dword [ebp + 16]
    fild dword [height]
    fld dword [half]
    fmul
    fdiv
    fsub
    fstp dword [ebp - 16]

    ; X dir
    fld dword [aspect]
    fld dword [ebp - 8]
    fld dword [yawC]
    fld dword [ebp - 12]
    fmul
    fmul
    fmul
    fld dword [ebp - 8]
    fld dword [yawS]
    fld dword [pitchS]
    fld dword [ebp - 16]
    fmul
    fmul
    fmul
    fadd
    fld dword [pitchC]
    fld dword [yawS]
    fmul
    fsub
    fstp dword [eax]

    ; Y dir
    fld dword [ebp - 8]
    fld dword [pitchC]
    fld dword [ebp - 16]
    fmul
    fmul
    fld dword [pitchS]
    fadd
    fstp dword [eax + 4]

    ; Z dir
    fld dword [naspect]
    fld dword [ebp - 8]
    fld dword [yawS]
    fld dword [ebp - 12]
    fmul
    fmul
    fmul
    fld dword [ebp - 8]
    fld dword [yawC]
    fld dword [pitchS]
    fld dword [ebp - 16]
    fmul
    fmul
    fmul
    fadd
    fld dword [pitchC]
    fld dword [yawC]
    fmul
    fsub
    fstp dword [eax + 8]

    mov esp, ebp
    pop ebp
    ret 4

ref:
    push ebp
    mov ebp, esp

    ; Load struct address
    mov eax, dword [ebp + 8]

    ; Fill struct
    fldpi
    fst dword [eax]
    fst dword [eax + 4]
    fstp dword [eax + 8]

    pop ebp
    ret 4
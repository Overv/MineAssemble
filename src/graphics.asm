;
; This file contains all functions related to rendering the world
;

[bits 32]

extern hFov
extern yawC, yawS, pitchC, pitchS

section .data

        ; Helpful constants
        width dd 320
        halfWidth dd 160
        height dd 200
        halfHeight dd 100
        aspect dd 1.6
        naspect dd -1.6
        two dd 2.0
        half dd 0.5
        doubleCircle dd 720.0

section .text

    global rayDir

    ; Takes screen space x and y and returns ray direction
    ; as (x, y, z) floats
rayDir:
    push ebp
    mov ebp, esp

    ; Allocate local variables (vFov, fov, clipX, clipY, length)
    sub esp, 20

    ; Load struct address (x, y, z)
    mov eax, dword [ebp + 8]

    ; Calculate vertical fov and fov constant from horizontal fov

    ; vFov = 2.0f * atanf(tanf(hFov / 720.0f * M_PI) * 320.0f / 200.0f);
    ; RPN: 2.0 hFov 720.0 / pi * tan width * height / atan mul
    fld dword [two]
    fld dword [hFov]
    fld dword [doubleCircle]
    fdiv
    fldpi
    fmul
    fptan
    fstp dword [eax] ; Dump 1 the tan instruction pushes for some reason
    fild dword [width]
    fmul
    fild dword [height]
    fdiv
    fld1 ; 1 required for atan (takes two parameters like atan2 in C)
    fpatan
    fmul
    fst dword [ebp - 4] ; vFov

    ; fov = tanf(0.5f * vFov);
    ; RPN: 0.5 vFov * tan
    fld dword [half]
    fmul
    fptan
    fstp dword [eax]
    fstp dword [ebp - 8] ; fov

    ; clip X = x / 160.0f - 1.0f
    ; RPN: x 160.0 / 1.0 -
    fild dword [ebp + 12] ; x parameter
    fild dword [halfWidth]
    fdiv
    fld1
    fsub
    fstp dword [ebp - 12] ; clipX

    ; clip Y = 1.0f - y / 100.0f
    ; RPN: 1.0 y 100.0 / -
    fld1
    fild dword [ebp + 16] ; y parameter
    fild dword [halfHeight]
    fdiv
    fsub
    fstp dword [ebp - 16] ; clipY

    ; X dir = 1.6f * fov * yawC * clipX + fov * yawS * pitchS * clipY - pitchC * yawS
    ; RPN: 1.6 fov yawC clipX * * * fov yawS pitchS clipY * * * + pitchC yawS * -
    fld dword [aspect]
    fld dword [ebp - 8] ; fov
    fld dword [yawC]
    fld dword [ebp - 12] ; clipX
    fmul
    fmul
    fmul
    fld dword [ebp - 8] ; fov
    fld dword [yawS]
    fld dword [pitchS]
    fld dword [ebp - 16] ; clipY
    fmul
    fmul
    fmul
    fadd
    fld dword [pitchC]
    fld dword [yawS]
    fmul
    fsub
    fstp dword [eax] ; X dir

    ; Y dir = fov * pitchC * clipY + pitchS
    ; RPN: fov pitchC clipY * * pitchS +
    fld dword [ebp - 8] ; fov
    fld dword [pitchC]
    fld dword [ebp - 16] ; clipY
    fmul
    fmul
    fld dword [pitchS]
    fadd
    fstp dword [eax + 4] ; Y dir

    ; Z dir = -1.6f * fov * yawS * clipX + fov * yawC * pitchS * clipY - pitchC * yawC
    ; RPN: naspect fov yawS clipX * * * fov yawC pitchS clipY * * * + pitchC yawC * -
    fld dword [naspect]
    fld dword [ebp - 8] ; fov
    fld dword [yawS]
    fld dword [ebp - 12] ; clipX
    fmul
    fmul
    fmul
    fld dword [ebp - 8] ; fov
    fld dword [yawC]
    fld dword [pitchS]
    fld dword [ebp - 16] ; clipY
    fmul
    fmul
    fmul
    fadd
    fld dword [pitchC]
    fld dword [yawC]
    fmul
    fsub
    fstp dword [eax + 8] ; Z dir

    ; Resulting direction is not normalized, but that doesn't matter
    ; for the raytracing algorithm

    mov esp, ebp
    pop ebp
    ret 4
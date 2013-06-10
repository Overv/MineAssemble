;
; This file contains all functions related to rendering the world
;

[bits 32]

global ray_dir, face_normal, tex_index

extern zero
extern hFov
extern yawC, yawS, pitchC, pitchS

section .data

        ; Helpful constants
        width dd 320
        halfWidth dd 160
        height dd 200
        halfHeight dd 100

        aspect dd 1.6
        neg_aspect dd -1.6

        texture_scale dd 16
        texture_size dd 15
        two dd 2.0
        half dd 0.5

        doubleCircle dd 720.0

section .text

        ; Takes screen space x and y and returns ray direction
        ; as (x, y, z) floats
    ray_dir:
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
        ; RPN: neg_aspect fov yawS clipX * * * fov yawC pitchS clipY * * * + pitchC yawC * -
        fld dword [neg_aspect]
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

    ; Returns normal of given block face (face, *nx, *ny, *nz)
    face_normal:
        ; Initialize nx, ny and nz to zero (no direction)
        mov eax, [esp + 8] ; nx
        mov dword [eax], 0
        mov eax, [esp + 12] ; ny
        mov dword [eax], 0
        mov eax, [esp + 16] ; nz
        mov dword [eax], 0

        ; Jump to face
        mov eax, [esp + 4] ; face
        jmp [.face_tbl + eax * 4]

        ; Face jump table
    .face_tbl:
        dd .face_left
        dd .face_right
        dd .face_bottom
        dd .face_top
        dd .face_back
        dd .face_front
    .face_left:
        mov eax, [esp + 8] ; nx = -1
        mov dword [eax], -1
        ret
    .face_right:
        mov eax, [esp + 8] ; nx = 1
        mov dword [eax], 1
        ret
    .face_bottom:
        mov eax, [esp + 12] ; ny = -1
        mov dword [eax], -1
        ret
    .face_top:
        mov eax, [esp + 12] ; ny = 1
        mov dword [eax], 1
        ret
    .face_back:
        mov eax, [esp + 16] ; nz = -1
        mov dword [eax], -1
        ret
    .face_front:
        mov eax, [esp + 16] ; nz = 1
        mov dword [eax], 1
        ret

    ; Returns index into texture image (pos, face)
    tex_index:
        ; First jump to face to determine (u, v) texture coordinates
        ; These variables will be pushed on the FPU stack
        mov eax, [esp + 16] ; face
        jmp [.face_tbl + eax * 4]

        ; Face jump table
    .face_tbl:
        dd .face_left
        dd .face_right
        dd .face_bottom
        dd .face_top
        dd .face_back
        dd .face_front
    .face_left:
    .face_right:
        fld dword [esp + 12] ; u = pos.z
        fld dword [esp + 8] ; v = pos.y
        jmp .uv_selected
    .face_bottom:
    .face_top:
        fld dword [esp + 4] ; u = pos.x
        fld dword [esp + 12] ; v = pos.z
        jmp .uv_selected
    .face_back:
    .face_front:
        fld dword [esp + 4] ; u = pos.x
        fld dword [esp + 8] ; v = pos.y

    .uv_selected:
        ; Invert v (1.0 - v)
        fld1
        fsubr

        ; Compute final integer index

        ; (int) (v * 15.0)
        fild dword [texture_size]
        fmul
        fistp dword [esp - 4] ; Store in temp variable

        ; (int) (u * 15.0) * 16
        fild dword [texture_size]
        fmul
        fistp dword [esp - 8]
        mov eax, [esp - 8]
        mul dword [texture_scale]

        ; Combine
        add eax, [esp - 4]

        ret
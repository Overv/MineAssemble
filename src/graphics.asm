;
; This file contains all functions related to rendering the world
;

[bits 32]

%include "constants.asm"

global ray_dir, face_normal, tex_index, ray_color, reticle_color, draw_frame

extern vga

extern hFov
extern yawC, yawS, pitchC, pitchS
extern playerPos

extern get_block
extern raytrace
extern sunDir

extern palette
extern tex_grass, tex_dirt, tex_grass_side

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

    ; vec3 rayDir(int x, int y)
    ; Takes screen space x and y and returns ray direction
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

    ; void face_normal(int face, int* nx, int* ny, int* nz)
    ; Returns normal of given block face
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

    ; int tex_index(vec3 pos, int face)
    ; Returns index into texture image
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

section .text

    ; byte ray_color(int x, int y, int z, vec3 pos, int tex, int face)
    ; Return color for given block and world details
    ray_color:
        push ebp
        mov ebp, esp
        push esi

        ; Allocate space for hit info struct
        sub esp, 32

        ; Store texture image index
        mov esi, [ebp + 32] ; tex

        ; Do lighting ray trace and request feedback to local hit info struct
        mov eax, ebp
        sub eax, 32
        push eax ; Pointer to hit info struct

        push dword [sunDir + 8] ; Ray direction vec3 (in reverse because stack grows downwards)
        push dword [sunDir + 4]
        push dword [sunDir + 0]

        push dword [ebp + 28] ; Ray start position vec3
        push dword [ebp + 24]
        push dword [ebp + 20]

        call raytrace

        add esp, 28

        ; If light ray hit something, shift to dark texture indices
        cmp byte [ebp - 32], 1 ; Check if bool 'hit' in struct set to 1
        jne .not_shadow
        add esi, 256
    .not_shadow:

        ; Texture lookup based on side
        cmp dword [ebp + 36], FACE_BOTTOM
        je .dirt
        cmp dword [ebp + 36], FACE_TOP
        je .grass

        ; If this block is at the top of the world, nothing is above it
        cmp dword [ebp + 12], WORLD_SY - 1
        je .grass_side

        ; In other cases, if there is another block above this block, sides are dirt
        push dword [ebp + 16] ; z

        mov eax, [ebp + 12]
        add eax, 1
        push eax ; y + 1

        push dword [ebp + 8] ; x

        call get_block

        add esp, 12

        cmp eax, BLOCK_DIRT
        je .dirt

        ; Grassy side texture
    .grass_side:
        movzx eax, byte [esi + tex_grass_side]
        jmp .finish

        ; Grass texture
    .grass:
        movzx eax, byte [esi + tex_grass]
        jmp .finish

        ; Dirt texture
    .dirt:
        movzx eax, byte [esi + tex_dirt]

    .finish:
        pop esi
        mov esp, ebp
        pop ebp

        ret

    ; byte reticle_color(byte col)
    ; Return semi-inverted color for given color to draw nice aim reticle
    reticle_color:
        push ebx

        ; Palette index
        mov eax, [esp + 8]

        ; Find RGB channels of palette color
        mov bl, [eax * 3 + palette + 0] ; R
        mov cl, [eax * 3 + palette + 1] ; G
        mov dl, [eax * 3 + palette + 2] ; B

        ; If blue channel has the max value, it's sky
        cmp dl, 63
        je .sky

        ; If green is more prevalent than blue, it's grass
        cmp cl, bl
        jg .grass

    .dirt:
        ; Return inverse color for dirt
        mov eax, 254
        jmp .finish

    .grass:
        ; Return inverse color for grass
        mov eax, 253
        jmp .finish

    .sky:
        ; Return inverse color for blue sky
        mov eax, 255

    .finish:
        pop ebx
        ret

    ; void draw_frame()
    ; Draw a single frame to the VGA frame buffer
    draw_frame:
        push esi
        push ebx
        push ebp
        mov ebp, esp

        ; Allocate space for ray dir vec3
        sub esp, 12

        ; Initialize local variables
        mov edi, 0 ; x
        mov ebx, 0 ; y
        mov ecx, [vga] ; pixel
        mov edx, 0 ; pixel index

        ; Draw a pixel
    .pixel:
        ; Calculate ray direction (ECX not used by ray_dir)
        push ebx
        push edi

        mov esi, ebp
        sub esi, 12
        push esi ; Pointer to local vec3

        call ray_dir

        ; No need to pop struct pointer, done by callee
        add esp, 8

        ; Perform ray trace
        push ecx
        push edx

        push dword 0 ; Not interested in hit info

        push dword [ebp - 4] ; Copy of ray dir vec3 (in reverse because stack grows downwards)
        push dword [ebp - 8]
        push dword [ebp - 12]

        push dword [playerPos + 8] ; Copy of playerPos vec3
        push dword [playerPos + 4]
        push dword [playerPos + 0]

        call raytrace

        add esp, 28

        pop edx
        pop ecx

        ; Assign pixel color
        mov [ecx], al

        ; Draw aim reticle
        cmp edi, 157
        jle .vertical_reticle
        cmp edi, 163
        jge .vertical_reticle
        cmp ebx, 100
        jne .vertical_reticle

        ; x > 157 && x < 163 && y == 100? Draw horizontal reticle line
        push ecx
        push edx
        push eax
        call reticle_color
        add esp, 4
        pop edx
        pop ecx

        mov [ecx], al

        jmp .no_reticle
    .vertical_reticle:
        cmp ebx, 97
        jle .no_reticle
        cmp ebx, 103
        jge .no_reticle
        cmp edi, 160
        jne .no_reticle

        ; y > 97 && y < 103 && x == 160? Draw vertical reticle line
        push ecx
        push edx
        push eax
        call reticle_color
        add esp, 4
        pop edx
        pop ecx

        mov [ecx], al
    .no_reticle:


        ; Update XY coordinates
        inc edi
        cmp edi, 320
        jl .next_pixel
        mov edi, 0
        inc ebx

        ; Go to next pixel
    .next_pixel:
        inc ecx
        inc edx
        cmp edx, 320 * 200
        jl .pixel

        mov esp, ebp
        pop ebp
        pop ebx
        pop esi

        ret
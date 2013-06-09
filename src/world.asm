;
; This file contains all functions related to the state of the world
;

[bits 32]

%include "constants.asm"

global init_world, set_block, get_block

global world, worldSX, worldSY, worldSZ
global sunDir

extern init_player, setBlock

section .data

    ; World block dimensions
    worldSX dd WORLD_SX
    worldSY dd WORLD_SY
    worldSZ dd WORLD_SZ
    worldSize dd WORLD_SX * WORLD_SY * WORLD_SZ

    ; Sun shadows
    sunDir dd SUN_DIR_X, SUN_DIR_Y, SUN_DIR_Z

section .bss

    ; World block array (1 byte per block)
    world resb WORLD_SX * WORLD_SY * WORLD_SZ
    world_end:

section .text
    
    ; Initialize default world blocks (flat grass)
    init_world:
        push edi
        push ebx

        ; First initialize everything to 0
        mov eax, world
    .empty_world:
        mov byte [eax], BLOCK_AIR
        inc eax
        cmp eax, world_end
        jne .empty_world

        ; Now fill bottom half with dirt
        ; Loop over x, y, z and set to BLOCK_DIRT
        ; Stop when y is at WORLD_SY / 2
        mov eax, 0
        mov ebx, 0
        mov ecx, 0
    .fill_x:
    .fill_y:
    .fill_z:
        ; Caller saved
        push eax
        push ecx

        ; Set block value
        push dword BLOCK_DIRT
        push ecx
        push ebx
        push eax
        call set_block
        add esp, 16

        pop ecx
        pop eax

        inc ecx
        cmp ecx, WORLD_SZ
        jne .fill_z
        mov ecx, 0 ; Reset inner z iterator

        inc ebx
        cmp ebx, WORLD_SY / 2
        jne .fill_y
        mov ebx, 0 ; Reset inner y iterator

        inc eax
        cmp eax, WORLD_SX
        jne .fill_x

        ; Perform player initialization
        call init_player

        pop ebx
        pop edi

        ret

    ; Get block value given x, y and z coordinates
    get_block:
        ; Compute index (x * worldSY * worldSZ + y * worldSZ + z)
        ; worldSZ can be factorized out to produce:
        ; (x * worldSY + y) * worldSZ + z

        mov eax, [esp + 4] ; x
        mul dword [worldSY]
        add eax, [esp + 8] ; y
        mul dword [worldSZ]
        add eax, [esp + 12] ; z

        ; Retrieve from world array;
        ; The other 3 bytes of eax are also zero'd
        movzx eax, byte [eax + world]

        ret

    ; Set block value given x, y, z and type parameters
    set_block:
        ; Compute index (x * worldSY * worldSZ + y * worldSZ + z)
        ; worldSZ can be factorized out to produce:
        ; (x * worldSY + y) * worldSZ + z

        mov eax, [esp + 4] ; x
        mul dword [worldSY]
        add eax, [esp + 8] ; y
        mul dword [worldSZ]
        add eax, [esp + 12] ; z

        ; Assign to world array
        mov edx, [esp + 16] ; type (can't mov from memory -> memory)
        mov byte [eax + world], dl

        ret
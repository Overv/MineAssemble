;
; This file contains all data relevant to the entire system
;

[bits 32]

; Configuration constants

%define WORLD_SX 32
%define WORLD_SY 32
%define WORLD_SZ 32

%define SUN_DIR_X 1.0
%define SUN_DIR_Y 3.0
%define SUN_DIR_Z 1.0

%define HOR_FOV 90.0

; Global variables

global vga

global width, height, aspect, naspect, half

global worldSX, worldSY, worldSZ
global sunDir
global hFov
global world

global playerPos
global pitch, pitchS, pitchC
global yaw, yawS, yawC

global dPitch, dYaw, velocity

section .data
        ; VGA buffer address
        vga dd 0xa0000

        ; Helpful constants
        width dd 320
        height dd 200
        aspect dd 1.6
        naspect dd -1.6
        half dd 0.5

        ; World block dimensions
        worldSX dd WORLD_SX
        worldSY dd WORLD_SY
        worldSZ dd WORLD_SZ

        sunDir dd SUN_DIR_X, SUN_DIR_Y, SUN_DIR_Z

        ; Horizontal field-of-view
        hFov dd HOR_FOV

        ; Input/update related (e.g. dPitch = pitch change per second)       
        dPitch dd 0.0
        dYaw dd 0.0
        velocity dd 0.0, 0.0, 0.0

section .bss
        ; World block array (1 byte per block)
        world resb WORLD_SX * WORLD_SY * WORLD_SZ

        ; Player position (3 floats)
        playerPos resd 3

        ; Player orientation (pitch and yaw + cached sine/cosine)
        pitch resd 1
        pitchS resd 1
        pitchC resd 1

        yaw resd 1
        yawS resd 1
        yawC resd 1
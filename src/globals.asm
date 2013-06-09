;
; This file contains all data not belonging to any particular module
;

[bits 32]

%include "constants.asm"

; Global variables

global vga

global hFov
global world

global playerPos
global pitch, pitchS, pitchC
global yaw, yawS, yawC

global dPitch, dYaw, velocity

section .data
        ; VGA buffer address
        vga dd 0xa0000

        ; Horizontal field-of-view
        hFov dd HOR_FOV

        ; Input/update related (e.g. dPitch = pitch change per second)       
        dPitch dd 0.0
        dYaw dd 0.0
        velocity dd 0.0, 0.0, 0.0

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
;
; This file contains all data not belonging to any particular module
;

[bits 32]

%include "constants.asm"

; Global variables

global vga

global half_pi, neg_half_pi, zero

global hFov
global dPitch, dYaw, velocity

global colTolerance

section .data
        ; VGA buffer address
        vga dd 0xa0000

        ; Helpful constants
        half_pi dd 1.57
        neg_half_pi dd -1.57
        zero dd 0.0

        ; Horizontal field-of-view
        hFov dd HOR_FOV

        ; Input/update related (e.g. dPitch = pitch change per second)       
        dPitch dd 0.0
        dYaw dd 0.0
        velocity dd 0.0, 0.0, 0.0

        ; Collision tolerance
        colTolerance dd 0.1
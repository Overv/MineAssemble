;
; This file contains all data not belonging to any particular module
;

[bits 32]

%include "constants.asm"

; Global variables

global vga

global zero, half_pi, nhalf_pi

global hFov

global dPitch, dYaw, velocity

section .data
        ; VGA buffer address
        vga dd 0xa0000

        ; Helpful constants
        zero dd 0.0
        half_pi dd 1.57
        nhalf_pi dd -1.57

        ; Horizontal field-of-view
        hFov dd HOR_FOV

        ; Input/update related (e.g. dPitch = pitch change per second)       
        dPitch dd 0.0
        dYaw dd 0.0
        velocity dd 0.0, 0.0, 0.0
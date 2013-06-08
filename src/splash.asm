[bits 32]

global show_splash

extern splashBitmap
extern keys

%define KEY_ENTER 0x1C

section .text

    show_splash:
        push ecx
        push esi
        push edi

        mov ecx, 64000 ; 320 x 200 pixels
        mov esi, splashBitmap ; Source address
        mov edi, 0xa0000 ; Destination address (VGA buffer)

    write_pixel:
        sub byte [esi], 0x30 ; Subtract '0' from value
        movsb
        dec ecx
        cmp ecx, 0
        jne write_pixel

        ; Ignore first ENTER press (leftover from GRUB menu)
    ignore_grub_enter:
        cmp byte [keys + KEY_ENTER], 0
        je ignore_grub_enter
        mov byte [keys + KEY_ENTER], 0

        ; Wait for user to press ENTER to continue
    wait_enter2:
        cmp byte [keys + KEY_ENTER], 0
        je wait_enter2

        pop edi
        pop esi
        pop ecx

        ret
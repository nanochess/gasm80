	;
	; Hello World for Colecovision
	;
	; by Oscar Toledo G.
	; https://nanochess.org/
	;
	; Creation date: May/03/2024.
	;

TURN_OFF_SOUND:	equ $1fd6
WRITE_REGISTER:	equ $1fd9
FILL_VRAM:	equ $1f82
WRITE_VRAM:	equ $1fdf

	org $8000

	db $55,$aa
	dw 0
	dw 0
	dw 0
	dw 0
	dw START

	jp 0	; rst $08
	jp 0	; rst $10
	jp 0	; rst $18
	jp 0	; rst $20
	jp 0	; rst $28
	jp 0	; rst $30
	jp 0	; rst $38
	retn	; NMI handler
	nop

START:
	call TURN_OFF_SOUND
        ld bc,$0000
        call WRITE_REGISTER
        ld bc,$0180
        call WRITE_REGISTER
        ld bc,$0206
        call WRITE_REGISTER
        ld bc,$0380
        call WRITE_REGISTER
        ld bc,$0400
        call WRITE_REGISTER
        ld bc,$0536
        call WRITE_REGISTER
        ld bc,$0607
        call WRITE_REGISTER
        ld bc,$0704
        call WRITE_REGISTER
        ld bc,$03F0	; Size of characters table.
        ld de,$00E8 	; Pointer to VRAM.
        ld hl,$158B     ; Direct access to Colecovision ROM
        call WRITE_VRAM	; Copy character table.
        ld hl,$2000	; Color table.
        ld de,32 	; All characters (32 bytes)
        ld a,$F4	; White on blue background.
        call FILL_VRAM	; Fill VRAM.
        ld hl,$1B00	; Sprite attribute table.
        ld de,128	; 128 bytes.
        ld a,$D1	; Disable all.
        call FILL_VRAM	; Fill VRAM.
        ld hl,$1800	; Screen address in VRAM.
        ld de,769
        ld a,$20	; Space character.
        call FILL_VRAM	; Fill VRAM.
        ld bc,22	; 22 characters.
        ld de,$1945 	; Screen address in VRAM.
        ld hl,message	; Pointer to message.
        call WRITE_VRAM	; Copy data to VRAM.
        ld bc,$01C0	; Enable the screen.
        call WRITE_REGISTER
        jr $

message:
	 db "---- Hello world! ----"


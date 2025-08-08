;***************
; EEPROM-routines
; for 93C46 (128 bytes as 64 16-bit words)
;
; created : 11.05.95
; last modified :
; 
; 16.02.96      leaner (thanks to Harry)
; 12.03.96      test for busy after write and erase (well, Harry ;)) )
; 22.08.97      ported to ra65 for use with cc65
; 02.12.97      added xref for the new ra65
;
;
; (c) 1995..97 Bastian Schick
; CS    = A7 (18)
; CLK   = A1 (11)
; DI/DO = AUDIN (32)
;
;And now how to contact the EEPROM :
;
;CARD
;PORT               ----\/----      93C46(SMD too)
;(18)  A7   --------| CS     |- +5V
;(11)  A1   --------| CLK    |- NC
;               +---| DI     |- NC
;(32) AUDIN ----+---| DO     |- GND
;                   ----------

; Same as eeprom code except A8 is the CS for the MCU (emulated eeprom)
        .export         _lynx_eeread_BV
        .export         _lynx_eewrite_BV
        .export         _lynx_eeread_93c46
        .export         _lynx_eewrite_93c46
        .export		_FloatAUDIN
        .export		_smoldelay
        .export		_longdelay
        .export		_DriveAUDIN

	.importzp	ptr1
	.import		popax

        .include        "lynx.inc"

; -------------------
; EEPROM command list
EE_C_WRITE      =    $40
EE_C_READ       =    $80
EE_C_ERASE      =    $C0
EE_C_EWEN       =    $30
EE_C_EWDS       =    $00


_smoldelay:
	lda #$FF
	ldx #$40
sd1:
	dea
	bne sd1
	dex
	bne sd1

	rts


_longdelay:
	lda #$FF
	ldx #$FF
	ldy #$20
sd2:
	dea
	bne sd2
	dex
	bne sd2
	dey
	bne sd2
	rts


; ------------------------------------------------------------------------
; unsigned __fastcall__ lynx_eeread_93c46(unsigned char cell);
; /* Read a 16 bit word from the given address */
;
_lynx_eeread_BV:
        and #$3f
        ora #EE_C_READ
        jsr EE_Send9BitBV
        jsr EE_Read16BitBV
        lda ptr1
        ldx ptr1+1
        rts
        

;***************
; write word to MCU
; void __fastcall__ lynx_eewrite_93c46(unsigned int addr, unsigned int val);
_lynx_eewrite_BV:
        sta ptr1
        stx ptr1+1
        jsr popax
        and #$3f
        ora #EE_C_WRITE
        jsr EE_Send9BitBV
        jsr EE_Send16Bit        ; Sends ptr1 that contains val
        ;jsr E_waitBV
        rts
        
        
        
	
; ------------------------------------------------------------------------
; unsigned __fastcall__ lynx_eeread_93c46(unsigned char cell);
; /* Read a 16 bit word from the given address */
;
_lynx_eeread_93c46:
        and #$3f
        ora #EE_C_READ
        jsr EE_Send9Bit
        jsr EE_Read16Bit
        lda ptr1
        ldx ptr1+1
        rts

;***************
; reads EEPROM-word to ptr1
; A,Y destroyed
EE_Read16Bit:
        lda #$a
        sta IODIR       ; set AUDIN to Input
        clc
        stz ptr1
        stz ptr1+1
        ldy #15
EEloop1:
; CLK = 1
        stz RCART0
        stz RCART0
; CLK = 0
        stz RCART0
        stz RCART0

        lda IODAT
        and #$10        ; mask bit
        adc #$f0        ; C=1 if A=$10
        rol ptr1
        rol ptr1+1      ; shifts 0 to Carry
        dey
        bpl EEloop1

        ldx #$1a
        stx IODIR       ; set AUDIN for output
;EE_SET_CS_LOW

        ldx #3
        stx SYSCTL1
        dex
        stx SYSCTL1

        rts
;***************
; reads EEPROM-word to ptr1
; A,Y destroyed
EE_Read16BitBV:
        lda #$a
        sta IODIR       ; set AUDIN to Input
        clc
        stz ptr1
        stz ptr1+1
        ldy #15
EEloop1BV:
; CLK = 1
        stz RCART0
        stz RCART0
; CLK = 0
        lda IODAT
        stz RCART0
        stz RCART0


        and #$10        ; mask bit
        adc #$f0        ; C=1 if A=$10
        rol ptr1
        rol ptr1+1      ; shifts 0 to Carry
        dey
        bpl EEloop1BV

        ldx #$1a
        stx IODIR       ; set AUDIN for output
;EE_SET_CS_LOW

        ldx #3
        stx SYSCTL1
        dex
        stx SYSCTL1

        rts

;***************
; write word to EEPROM
; void __fastcall__ lynx_eewrite_93c46(unsigned int addr, unsigned int val);
_lynx_eewrite_93c46:
        sta ptr1
        stx ptr1+1
        lda #EE_C_EWEN
        jsr EE_Send9Bit
        jsr popax
        and #$3f
        ora #EE_C_WRITE
        jsr EE_Send9Bit
        jsr EE_Send16Bit        ; Sends ptr1 that contains val

EE_wait:
; EE_SET_CS_HIGH

        ldx #63
EEloop:
        stz RCART0
        stz RCART0
        dex
        bpl EEloop

        lda #$0A
        sta IODIR       ; AUDIN to input
        lda #$10
EE_wait1:
        bit IODAT       ; 'til ready :D0-read is /D0-written
        beq EE_wait1
        lda #$1a        ; AUDIN to output
        sta IODIR
        lda #EE_C_EWDS
;       bra EE_Send9Bit ; fall into
;***************
; send A via I2C
; A,Y destroyed
;***************


_FloatAUDIN:
        lda #$0A
        sta IODIR 
	rts
	
_DriveAUDIN:
        lda #$1a        ; AUDIN to output
        sta IODIR
	rts

EE_Send9Bit:
; EE_SET_CS_LOW
        ldy #3
        sty SYSCTL1
        dey
        sty SYSCTL1
; EE_SET_CS_HIGH

        ldy #63
EEloop2:
        stz RCART0
        stz RCART0
        dey
        bpl EEloop2

        ldy #8
        sec             ; start bit
        ror A
        ror A
        ror A
        ror A           ; bit 8 at pos. 4
EEloop3:
        tax
        and #$10
        ora #$b
        sta IODAT
; CLK = 1
        stz RCART0
        stz RCART0
; CLK = 0
        stz RCART0
        stz RCART0
        txa
        rol A
        dey
        bpl EEloop3
        rts


EE_Send9BitBV:
; EE_SET_CS_LOW
        ldy #3
        sty SYSCTL1
        dey
        sty SYSCTL1
; EE_SET_CS_HIGH

        ldy #127
EEloop2BV:
        stz RCART0
        stz RCART0
        dey
        bpl EEloop2BV

        ldy #8
        sec             ; start bit
        ror A
        ror A
        ror A
        ror A           ; bit 8 at pos. 4
EEloop3BV:
        tax
        and #$10
        ora #$b
        sta IODAT
        NOP
        NOP
        NOP
        NOP
; CLK = 1
        stz RCART0
        stz RCART0
; CLK = 0
        stz RCART0
        stz RCART0
        txa
        rol A
        dey
        bpl EEloop3BV
        rts
;***************


;***************
; send ptr1 to EEPROM
EE_Send16Bit:
        lda ptr1+1

        ror A
        ror ptr1
        ror A
        ror ptr1
        ror A
        ror ptr1

        ldy #15
EEloop5:
        tax
        and #$10
        ora #$b
        sta IODAT
        NOP
        NOP
        NOP
        NOP
; CLK = 1
        stz RCART0
        stz RCART0
; CLK = 0
        stz RCART0
        stz RCART0
        txa
        rol ptr1
        rol A
        dey
        bpl EEloop5

; EE_SET_CS_LOW
        ldx #3
        stx SYSCTL1
        dex
        stx SYSCTL1
        rts



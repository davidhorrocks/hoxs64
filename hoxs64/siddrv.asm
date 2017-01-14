#include "c64def.asm"

	.text
	;*=$2000
start
	jmp initapp

	; SID file header
sfID			.dsb 4
sfVersion		.dsb 2
sfDataOffset	.dsb 2
sfLoadAddress	.dsb 2
sfInitAddress	.dsb 2
sfPlayAddress	.dsb 2
sfSongs			.dsb 2
sfStartSong		.dsb 2
sfSpeed			.dsb 4
sfName			.dsb 32
sfAuthor		.dsb 32
sfCopyright		.dsb 32
sfFlags			.dsb 2
sfStartPage		.dsb 1
sfPageLength	.dsb 1
sfReserved		.dsb 2

songnumber		.byt $00
keydown_space	.byt $00
keydown_comma	.byt $00
keydown_dot		.byt $00

initapp .(
	lda #15
	sta SID + SID_SR2
	ldx sfStartSong
	beq j1
	dex
j1
	stx songnumber
	jsr initdisplay
	.)

startsong
	lda sfPlayAddress
	bne customplay
	lda sfPlayAddress+1
	bne customplay
autoplay
	sei
	jsr initsid
	jsr initsong
	cli
	jmp loop4ever

customplay .(
	jsr removekernal
	jsr initsid
	jsr initsong
	jsr removekernal	

	lda sfFlags
	and #$0c
	cmp #$08
	beq speedCIA	;NTSC
	
	ldx songnumber
	cpx #31
	bcc j1		;songnumber < 31
	ldx #31
j1
	txa
	lsr
	lsr
	lsr
	tay			;byte index into sfSpeed
	txa
	and #7		;bit number in speed byte
	tax
	lda #0
	sec
j2
	rol
	dex
	bpl j2
	and	sfSpeed,y
	beq speedVBL	;50hz VBL speed
speedCIA
	jsr initcia
	jmp entermainloop
speedVBL
	jsr initvbl
entermainloop
	cli
	jmp loop4ever
	.)
	

removekernal
	sei
	lda #$2f
	sta $00
	lda #%00000101 ; select just RAM and i/o
	sta $01
	rts

loop4ever 
	.(
	jsr checkkey_space
	and keydown_space
	beq j1	; branch if space key is not pressed
	jmp nextsong
j1
	jsr checkkey_comma
	and keydown_comma
	beq j2	; branch if comma key is not pressed
	jmp prevsong
j2
	jsr checkkey_dot
	and keydown_dot
	beq j3	; branch if dot key is not pressed
	jmp nextsong
j3
	jmp loop4ever

	.)

initsid
	.(
	lda #0
	ldx #SID_2SUSREL
j1
	sta SID, x
	dex
	bpl j1

	lda #8
	sta SID + SID_0SR;
	sta SID + SID_1SR;
	sta SID + SID_2SR;

	lda #$00
	sta SID + SID_FILTL
	sta SID + SID_FILTH

	ldy #2
	ldx #0
j2
	dex
	bne j2
	dey
	bne j2

	lda #0
	sta SID + SID_0SR;
	sta SID + SID_1SR;
	sta SID + SID_2SR;

	lda #15
	sta SID + SID_SR2
	rts
	.)

checkkey_space .(
	lda #$7f ;
	jsr checkkey
	and #$10		; non zero if space key is down
	tax
	eor keydown_space
	stx keydown_space
	rts
	.)

checkkey_comma	 .(
	lda #$df ;
	jsr checkkey
	and #$80		; non zero if comma key is down
	tax
	eor keydown_comma ; A!=0 key state change
	stx keydown_comma ; X!=0 key is down
	rts
	.)

checkkey_dot	 .(
	lda #$df ;
	jsr checkkey
	and #$10		; non zero if dot key is down
	tax
	eor keydown_dot ; A!=0 key state change
	stx keydown_dot ; X!=0 key is down
	rts
	.)

checkkey .(
	sei
	sta CIA1 + CIA_DRA ; should select just one row 
j1
	lda CIA1 + CIA_DRB ; read the columns
	cmp CIA1 + CIA_DRB
	bne j1
	cli
	eor #$ff
	rts
	.)
	
nextsong .(
	ldx songnumber
	inx
	cpx sfSongs
	bcc j2
	ldx #0
	stx songnumber
	jmp startsong
j2
	stx songnumber
	jmp startsong
	.)

prevsong .(
	ldx songnumber
	bne j2
	ldx sfSongs
j2
	dex
	stx songnumber
	jmp startsong
	.)

initsong .(
	lda #>initsong_return
	pha	
	lda #<initsong_return
	pha
	lda songnumber
	jmp (sfInitAddress)
initsong_return
	nop
	rts
	.)

initvbl 
	sei
	lda #<nmistart
	sta $FFFA
	lda #>nmistart
	sta $FFFB

	lda #<irqstart
	sta $FFFE
	lda #>irqstart
	sta $FFFF

	; set raster compare to $000
	lda #0
	sta VIC + VIC_IRQ_RASTER
	lda VIC + VIC_SR1
	and #$7f
	sta VIC + VIC_SR1


	lda #$1f
	sta CIA1 + CIA_ICR ; disable cia1 interrupts
	sta CIA2 + CIA_ICR ; disable cia2 interrupts
	lda CIA1 + CIA_ICR ; clear cia1 pending interrupts
	lda CIA2 + CIA_ICR ; clear cia2 pending interrupts

	lda #$01
	sta VIC + VIC_IMR ;enable VBL interrupt
	lda #$0f
	sta VIC + VIC_IRR
	rts

initcia
	sei
	lda #<nmistart
	sta $FFFA
	lda #>nmistart
	sta $FFFB

	lda #<cirqstart
	sta $FFFE
	lda #>cirqstart
	sta $FFFF

	lda #$1f
	sta CIA1 + CIA_ICR ; disable cia1 interrupts
	sta CIA2 + CIA_ICR ; disable cia2 interrupts
	lda CIA1 + CIA_ICR ; clear cia1 pending interrupts
	lda CIA2 + CIA_ICR ; clear cia2 pending interrupts

	lda #$81
	sta CIA1 + CIA_ICR ; enable cia1 timer a interrupts


	lda #$00
	sta VIC + VIC_IMR ;disable VIC interrupts
	lda #$0f
	sta VIC + VIC_IRR
	rts

irqstart .(
	sei
	pha
	txa
	pha
	tya
	pha
	lda CIA1 + CIA_ICR 
	lda CIA2 + CIA_ICR
	lda VIC + VIC_IRR
	sta VIC + VIC_IRR
	and #$01
	beq j1

	lda #>j1
	pha	
	lda #<j1
	pha

	jmp (sfPlayAddress)
j1
	nop
	jsr removekernal	
	pla
	tay
	pla
	tax
	pla
	rti
	.)

cirqstart .(
	sei
	pha
	txa
	pha
	tya
	pha
	lda VIC + VIC_IRR
	sta VIC + VIC_IRR

	lda CIA2 + CIA_ICR
	lda CIA1 + CIA_ICR 

	and #$01
	beq j1

	lda #>j1
	pha	
	lda #<j1
	pha

	jmp (sfPlayAddress)
j1
	nop
	jsr removekernal	
	pla
	tay
	pla
	tax
	pla
	rti
	.)

nmistart
	pha
	lda CIA2 + CIA_ICR
	pla
	rti


initdisplay .(
	jsr clearscreen
	lda #1
	jsr clearcolormap
	jsr setdisplay
	jsr printtitle1
	jsr printtitle2
	rts
	.)


clearscreen .(
	lda #$20
	ldx #$FA
j1
	sta $03ff,x
	sta $04F9,x
	sta $05F3,x
	sta $06ED,x
	dex
	bne j1
	rts
	.)

clearcolormap .(
	ldx #$FA
j1
	sta $D7FF,x
	sta $D8F9,x
	sta $D9F3,x
	sta $DAED,x
	dex
	bne j1
	rts
	.)


setdisplay .(
	lda #0
	sta VIC + VIC_EXTCOL
	sta VIC + VIC_BCKCOL0
	rts
	.)

printtitle1 .(
	ldx #$00
j2
	lda title1,x
	beq j1
	sta $0400+7,x
	inx
	jmp j2
j1
	rts
	.)

printtitle2 .(
	ldx #$00
j2
	lda title2,x
	beq j1
	sta $0400+40*3+2,x
	inx
	jmp j2
j1
	rts
	.)
	
	;title	 "HOXS64 SID PLAYER V0.00"
	;title	 "PRESS SPACE , AND . TO CHANGE SONG"
title2	.byt 16,18,5,19,19,32,19,16,1,3,5,32,44,32,1,14,4,32,46,32,20,15,32,3,8,1,14,7,5,32,20,18,1,3,11,0
title1	.byt 8,15,24,19,54,52,32,19,9,4,32,16,12,1,25,5,18,32,22,48,46,48,53,0

filestart org 0
romstart_page0
	put 0
	phase 0xc000
	bra .
	
	dephase	0xc000

	
romstart_page1	
	put 0x4000
	phase 0xc000	
	
VIA0_B	equ 0x8000
VIA0_A	equ	0x8001
VIA0_DDRB equ 0x8002
VIA0_DDRA equ 0x8003
VIA0_IFR equ 0x800d
VIA0_PCR equ 0x800c
VIA0_ACR equ 0x800b
VIA0_SR equ 0x800a

VIA1_IFR equ 0x801d
VIA1_PCR equ 0x801c
VIA1_ACR equ 0x801b
VIA1_SR equ 0x801a
VIA1_DDRB equ 0x8012
VIA1_DDRA equ 0x8013
VIA1_B	equ 0x8010
VIA1_A	equ	0x8011

VIA_PAGE equ 0x80

SD_WR equ VIA1_SR
SD_RD equ VIA0_SR

XAMH equ 0x24
XAML equ 0x25
STH equ 0x26
STL equ 0x27
H equ 0x28
L equ 0x29
YSAV equ 0x2a
MODEH equ 0x2c
MODEL equ 0x2d
IN equ 0x200

STACK equ 0x400 ;1024 bytes of stack


inx macro
	leax 1,x
	endm
dex macro
	leax -1,x
	endm
iny macro
	leay 1,y
	endm
dey macro
	leay -1,y
	endm

main
	lda #VIA_PAGE
	tfr a, dp
	setdp VIA_PAGE ;VIA pAGE page

	lda #$ff
	sta VIA1_A
	sta VIA1_DDRA

	lda #$ff
	sta VIA0_DDRB
	lda #$0
	sta VIA0_DDRA
	lda #$f
	sta VIA1_PCR
	lda #$a
	sta VIA0_PCR	;set via0 to read handhskae?
	
	
	
	setdp $0 ;zero page
	
	ldx #STACK ;load stack value
	tfr x, s ;init stack
	
		
	lda #$ffff
	jsr spin	;wait for SIO startup
	lda #$ffff
	jsr spin
	
	ldx #sd_str
	jsr putstr
	
	jsr sd_init

	


hello_str fcc "Hello from 6809!", 10, 13, "Welcome to WOZMON!",10,13,0
sd_str fcc "SD INIT",10,13,0
hexlookup fcc "0123456789ABCDEF"

prbyte:	
	pshs a
	lsra
	lsra
	lsra
	lsra
	bsr prhex
	puls a
prhex	anda #$f
	ora #$30
	cmpa #$3a
	blo echo
	adda #$7
echo:
	pshs a
	jsr putchar
	puls a
	rts
	

pr32 ;prints a:b:x 32 bit int
	pshs b,x
	bsr prbyte ;print 32:24
	puls a ;oldb contents now at a
	bsr prbyte ;print 24:16
	puls a ;x 16:8 now at a
	bsr prbyte
	puls a ;x 8;0 now at a
	bsr prbyte
	rts

pr16 ;prints a:b 16 bit int
	pshs b
	bsr prbyte ;print 16:8
	puls a ;oldb contents now at a
	bsr prbyte ;print 8:0

	rts

send_dummy	lda #$2
	sta VIA0_B
	bsr wait_ack
	rts

	
	
putstr	lda ,x+	;load x and inc
	beq 1f	;leave if zero
	bsr putchar
	bra putstr
1		rts		

getchar 	;returns char in A
	lda #$3	;getchar cmd
	sta VIA0_B
	bsr wait_ack
	bsr wait_in
	lda VIA0_A
	rts
	
putchar pshs a
	lda #$1		putchar cmd
	sta VIA0_B	A=input hcar
	bsr wait_ack
	puls a 
	sta VIA0_B
	bsr wait_ack
	rts



wait_in 
	lda #0b10	;clear ifr flag
	sta VIA0_IFR
	
1	lda VIA0_IFR	;load from interrupt flag reg
	bita #0b10	;is ca1 flag set
	beq 1b	;b if flag not set
	rts

wait_ack lda  #$d
	sta VIA1_PCR
	
	lda #0b10	;clear ifr flag
	sta VIA1_IFR
	
1	lda VIA1_IFR	;load from interrupt flag reg
	bita #0b10	;is ca1 flag set
	beq 1b	;b if flag not set
	lda #$f
	sta VIA1_PCR
	rts
	

spin ;x = time to spin for
	mul
	dex
	bne spin
	rts
cursor  
	fdb 0b111000,00000000,00000000,00000000
	fdb 0b111111,00000000,00000000,00000000,
	fdb 0b111000,0b111000,00000000,00000000,
	fdb 0b111000,0b000111,00000000,00000000,
	fdb 0b111000,0b000000,0b111000,00000000,
	fdb 0b111000,0b000000,0b000111,00000000,
	fdb 0b111000,0b000000,0b000000,0b111000,
	fdb 0b111000,0b000000,0b000111,0b111111,
	fdb 0b111000,0b111000,0b000111,0b000000,
	fdb 0b111111,0b000111,0b000000,0b111000,
	fdb 0b111000,0b000111,0b000000,0b111000,
	fdb 0b000000,0b000000,0b111000,0b000111,
	fdb 0b000000,0b000000,0b111000,0b000111,
	fdb 0b000000,0b000000,0b000111,0b111000,
	fdb 0b000000,0b000000,0b000000,0b000000,
	fdb 0b000000,0b000000,0b000000,0b000000


	INCLUDE	"sdcard.s"
	INCLUDE "mbr.s"
	INCLUDE "fat.s"

	dephase 0xc000
reset_vector	put 0x7ffe
	fdb main
	
	

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
VIA0_T1L equ 0x8006
VIA0_T1H equ 0x8007
VIA0_IER equ 0x800e
VIA0_T1CL equ 0x8004
VIA0_T1CH equ 0x8005


VIA1_IFR equ 0x801d
VIA1_PCR equ 0x801c
VIA1_ACR equ 0x801b
VIA1_SR equ 0x801a
VIA1_DDRB equ 0x8012
VIA1_DDRA equ 0x8013
VIA1_B	equ 0x8010
VIA1_A	equ	0x8011
VIA1_T1L equ 0x8016
VIA1_T1H equ 0x8017
VIA1_IER equ 0x801e
VIA1_T1CL equ 0x8014
VIA1_T1CH equ 0x8015

VIA_PAGE equ 0x80

SD_WR equ VIA1_SR
SD_RD equ VIA0_SR





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
	orcc #$50 ;disable interrupts

	lda #VIA_PAGE
	tfr a, dp
	setdp VIA_PAGE ;VIA pAGE page

	lda #$ff
	sta VIA1_A
	sta VIA1_DDRA

	lda #$ff
	sta VIA0_DDRB
	clra
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
	lda #$ffff
	jsr spin
	lda #$ffff
	jsr spin
	
	ldx #sd_str
	jsr putstr
	
		
	
	
	jsr sd_init
	

	jsr bmalloc_init

	ldx #$0
	ldy #$0
	jsr setc
	
	jsr gui_init

	
	leas -64, s; reserve 64 bytes on stack for own use
	;s: queue_ptr
	
	ldb #1
	jsr bmalloc ; allocate 256 bytes 
	;FIXME: multi bank switching stuff to be dealt with
	stx ,s
	
	ldd #60
	std ,--s ;ysz
	ldd #40
	std ,--s ;xsz
	ldd #0
	std ,--s ;y
	std ,--s ;x
	ldd #file_man_str
	std ,--s
	ldy #25 ;total queue size = 64 bytes, 25 ents
	;x already filled with address
	
	
	
	jsr gui_init_queue
	
	leas 10,s ;pop 
	
	ldx ,s
	leax 64, x ; our toolbar object stored in the same block
	
	leas -8, s ; 4 args on stack
	ldd #40 ;toolbar width is 40
	std 6,s
	ldd #26 ;26 menus, total size 63bytes
	std 4,s
	ldd #0 ;x y zero
	std ,s
	std 2,s
	jsr gui_toolbar_constructor
	
	leas 8,s ;stack cleanup
	
	ldx ,s
	leax 128,x ;menu object stored in same block, 32 bytes
	ldy #file_menu_str
	ldd #12
	pshs d
	jsr gui_menu_constructor
	
	leas 2,s ;pop off
	
	ldx ,s
	leax 64, x ;toolbar 
	leay 64, x ;menu
	jsr gui_toolbar_add_menu
	
	
	ldx ,s ; queue address in x
	leay 64,x ;widget at 64 bytes after queue 
	jsr gui_queue_add_widget
	
		

	ldx ,s
	jsr gui_draw_queue
	


	lda #0b11000000 ;Timer 1 interrupt enabled
	sta VIA1_IER
	
	
	lda #$ff
	sta VIA1_T1H
	lda #$ff
	sta VIA1_T1L ;/timer latch = 0xffff = 65535, watchdog called every 1/30 secs
	
	
	lda #$ff
	sta VIA1_T1CH
	lda #$ff
	sta VIA1_T1CL


	lda VIA1_ACR
	anda #$3f ;continuous interrupts from timer 1
	ora #$40 ;enable free running. do not outupt to pb7
	sta VIA1_ACR
	

	;enable hardware interrupts handling by clearing I bit
	andcc #$ef 

		ldx #gui_str 
	jsr putstr
	


	
	
5	sync
	bra 5b

file_man_str fcc "File manager",0
file_menu_str fcc "File",0
gui_str fcc "NO HANGS",0
hello_str fcc "Hello from 6809!", 10, 13, "Welcome to WOZMON!",10,13,0
malloc_str fcc "ADR",10,13,0
sd_str fcc "SD INIT",10,13,0
hexlookup fcc "0123456789ABCDEF"

prbyte:		;print A
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

;y = ty
;x = tx
setc
	lda #$5 ;CMD_SETC
	sta VIA0_B
	bsr wait_ack
	tfr y, d
	sta VIA0_B
	bsr wait_ack
	stb VIA0_B
	bsr wait_ack
	tfr x,d
	sta VIA0_B
	bsr wait_ack
	stb VIA0_B
	bsr wait_ack
	rts
	

;x = str ptr
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

getcharx
	lda #$7 ;CMD_GETCX
	sta VIA0_B
	bsr wait_ack
	bsr wait_in
	ldb VIA0_A
	
	pshs b
	bsr wait_in
	puls b

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

;a = fgbg byte
setfgbg
	pshs a
	lda #6 ;setfgbg
	sta VIA0_B 
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

wait_ack lda  #$d ;pull via1 ca2 low 
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


	INCLUDE "globals.s"
	INCLUDE	"sdcard.s"
	INCLUDE "mbr.s"
	INCLUDE "fat.s"
	INCLUDE "bmalloc.s"
	INCLUDE "gui.s"
	INCLUDE "int.s"

	dephase 0xc000
int_vector		put 0x7ff8 ;int vec
	fdb irq_handler
reset_vector	put 0x7ffe
	fdb main
	
	

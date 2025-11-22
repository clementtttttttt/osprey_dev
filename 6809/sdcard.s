sd_wait_spi
	lda VIA1_IFR
	bita #0b100 ;is shift register interrupt flag set
	bne sd_wait_spi
	lda #0b100
	sta VIA1_IFR ;unset flag
	rts

sd_tfr	macro		;A = byte to be sent to sd
					;flags = byte stuff
	sta SD_WR
	jsr sd_wait_spi
	lda SD_RD
	endm

sd_tfrb	macro		;A = byte to be sent to sd
					;flags = byte stuff
	stb SD_WR
	jsr sd_wait_spi
	ldb SD_RD
	endm

sd_send_cmdidx macro
	ora #$40 ;cmdbytes always have bit 6 set 
	sd_tfr
	endm
	
sd_sel	;lowers chip select
	
	lda VIA1_A
	anda #~(1<<4)
	sta VIA1_A
	rts
	
sd_read_cmdret_byte	;read 1 byte of return val from cmd, if sign flag setthen error
	ldx #$ffff ;timeout
1	dex
	beq 2f
	lda #$ff
	sd_tfr
	bmi 1b  ;loop if bit 7 set
	rts
2	orcc #0b1000
	rts
	
	
	
	
	
	
sd_desel ;raises chip select
	lda VIA1_A
	ora #(1<<4)
	sta VIA1_A
	rts

sd_send_cmd55
	lda #$ff ;alignment bytes
	sd_tfr 
	
	lda #55
	sd_send_cmdidx
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	
	jsr sd_read_cmdret_byte
	bita #$0b11111100
	lbne sd_prn_err
	
	lda #$ff
	sd_tfr 
	
	rts

;a:b:x = 32 bit in addr, y = read buf 
;A != 0x5 if error
;y = crc if success
sd_write_sect
	pshs a
	lda #$ff
	sd_tfr
	jsr sd_sel
	lda #$ff
	sd_tfr ;dummy bytes
	
	lda #24 ;cmd24, write 1 sector
	sd_send_cmdidx
	puls a ;pop back msbyte of addr
	sd_tfr ;send 32:24 of addr
	sd_tfrb ;send 24:16 of addr
	
	tfr x,d
	sd_tfr ;send 16:8 of addr
	sd_tfrb ;send 8:0 of addr
	
	lda #0 ; crc 0
	sd_tfr
	
	jsr sd_read_cmdret_byte
	bita #$0b11111100
	lbne sd_prn_err
	
	
	

	lda #$fe
	
	sd_tfr
	
	ldx #512 ;read 512 bytes
3 ;read loop
	
	sta ,y+ ;write into buffer
	sd_tfr
	dex
	bne 3b
	
1 ;wait for response
	lda #$ff
	sd_tfr
	cmpa #$ff
	beq 1b
	
	anda #$1f
	cmpa #$5
	bne 2f
	
4; wait for sdcard write op to finish
	
	lda #$ff
	sd_tfr
	beq 4b ;wait until not zero
	
	lda #5
2	;err jump
	jsr sd_desel
	
	ldb #$ff
	sd_tfrb
	
	
	rts

;a:b:x = 32 bit LBA addr, y = read buf 
;0,s = count (16 bit)
; u  sould be preserved

;A != 0xfe if error
sd_read_multi_sect
	pshs y

2
	ldy 4,s ;load sector count into y
	beq 3f ;quit
	
	dey
	sty 4,s ;decrement count and store back
	
	pshs a,b,x
	
	ldy 4,s ;restore buffer
	
	jsr sd_read_sect
	cmpa #$fe
	beq 1f ;quit if error
	
	ldy 4,s
	leay 512,y
	sty 4,s ;inc addr by 1 sect
	
	puls a,b,x
	
	
	inx
	bne 2b ;not zero = no carry
	addd #$1 ;do carry thing
	bra 2b
	
1	
	leas 4,s ; pop preserved regs
	rts
	
3	lda #$fe ;no error
	leas 2,s ;pop preserved stack addr
	rts
	
	
	

;a:b:x = 32 bit LBA addr, y = read buf 
;A != 0xfe if error
;y = crc if success
sd_read_sect
	pshs a
	lda #$ff
	sd_tfr
	jsr sd_sel
	lda #$ff
	sd_tfr ;dummy bytes
	
	lda #17 ;cmd17, read 1 sector
	sd_send_cmdidx
	puls a ;pop back msbyte of addr
	sd_tfr ;send 32:24 of addr
	sd_tfrb ;send 24:16 of addr
	
	tfr x,d
	sd_tfr ;send 16:8 of addr
	sd_tfrb ;send 8:0 of addr
	
	lda #0 ; crc 0
	sd_tfr
	
	jsr sd_read_cmdret_byte
	bita #$0b11111100
	lbne sd_prn_err
	
1	;wait_fe
	lda #$ff
	sd_tfr
	cmpa #$ff
	beq 1b ;wait while null
	cmpa #$fe
	bne 2f
	
	ldx #512 ;read 512 bytes ;;FIXME: non-512 byte sector size support
3 ;read loop
	
	lda #$ff
	sd_tfr
	sta ,y+ ;write into buffer
	dex
	bne 3b
	
	ldb #$ff
	sd_tfrb ;msb of crc
	lda #$ff
	sd_tfr ;lsb of crc
	

	lda #$fe

2 ;error, return early with response in a 	
	
	jsr sd_desel
	
	ldb #$ff
	sd_tfrb
	
	rts
sd_init	

	lda #0b11000 ;shift register shift out under phi2 contrl
	sta VIA1_ACR
	
	lda #0b01100 ;shift register shift in under ext clock
	sta VIA0_ACR

	lda #0b100
	sta VIA1_IFR ;unset flag
	
	lda VIA1_DDRA
	ora #(1<<4) ;CS pin is output
	sta VIA1_DDRA
	

	jsr sd_desel
	
	ldx #1000
	jsr spin
	
	ldx #10 ;80 dummy cycles for sync
1
	lda #$ff
	sd_tfr
	dex
	bne 1b
	
	jsr sd_sel
	lda #$ff 
	sd_tfr ;more bytes just in case
	lda #$ff 
	sd_tfr ;more bytes just in case
		
	
	jsr sd_desel

	lda #$ff 
	sd_tfr ;dummy init bytes
	lda #$ff 
	sd_tfr ;more bytes just in case
		
	
	jsr sd_sel
	lda #$ff 
	sd_tfr ;more bytes just in case
	lda #$ff 
	sd_tfr ;more bytes just in case
		
		
	lda #$0
	sd_send_cmdidx
	lda #$0 ;null bytes
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #$94 ;hard coded crc
	sd_tfr
	
	jsr sd_read_cmdret_byte
	;if error then reset
	lbmi sd_init
	
	bita #$0b11111100
	lbne sd_prn_err
	
	lda #$ff
	sd_tfr ;send dummy
	
	lda #$8 ;sd cmd 8 send_if_conds
	sd_send_cmdidx
	lda #$0
	sd_tfr
	lda #$0
	sd_tfr 
	lda #$1
	sd_tfr ;3.3v
	lda #$aa
	sd_tfr ;check pattern
	lda #$86
	sd_tfr ;crc
	
	jsr sd_read_cmdret_byte ;read r7 response
	bita #0b11111100
	lbne sd_prn_err
	
	lda #$ff
	sd_tfr
	lda #$ff
	sd_tfr
	lda #$ff
	sd_tfr
	
	lda #$ff
	sd_tfr 
	
	cmpa #$aa ;is check pattern good
	lbne sd_prn_err ;no

2 	;init loop
	jsr sd_send_cmd55
	
	
	
	lda #41 ;acmd41
	sd_send_cmdidx
	
	lda #$40
	sd_tfr 
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	lda #0
	sd_tfr
	
	lda #0 ;crc
	sd_tfr
	
	jsr sd_read_cmdret_byte
	bita #0b11111100
	lbne sd_prn_err
	bita #$ff
	bne 2b ;send stuff again if still in idle
	
	 
	
	
	jsr sd_desel
	
	ldx #sd_success_str
	jsr putstr
	
	
	 ;allocate buffer on stack
	leas -512,s
	
	
	tfr s,y
	
	
	ldd #0
	ldx #0
	
	jsr sd_read_sect
	
	lda #$0 
	tfr s,y ;load buf address in stack
	jsr mbr_read_part_lba
	
	tfr s,y ;load buf address, again
	pshs d,x ;push lba
	jsr sd_read_sect ;read fat block
	
	puls d,x ;restore lba
	tfr s,y; get buffer address
	
	jsr fat_get_root_lba ;read in root lba
	
	tfr s,y
	jsr sd_read_sect ;read first root dir sector
	
	ldd 28,s ;file size , 16:0
	exg a,b
	tfr d,x
	ldd 30,s ;file size, 32:16
	exg a,b
	
	jsr pr32
	

	leas 512,s


	rts
	
	
sd_prn_err: ;prints err, code in a
	pshs a
	ldx #sd_err_str
	jsr putstr
	puls a
	ldy #7 ;7 iterations
1	
	asla
	bpl 2f

	pshs a
	tfr y,d
	lda #0 
	aslb
	aslb; multiply by 4
	tfr d,x
	leax sd_errs,x ;load string addr in x 
	lda ,x+
	jsr putchar
	lda ,x+
	jsr putchar
	lda ,x+
	jsr putchar
	lda ,x+
	jsr putchar
	
	lda #10
	jsr putchar
	lda #13
	jsr putchar
	
	puls a
	
2
	dey
	bne 1b ; loop if not 0
	bra .
	rts
	
	
	
sd_err_str	fcc	"SD card error!",10,13,0
sd_success_str fcc "SD card init success",10,13,0
sd_errs	fcc	"IDLE"
		fcc	"ERST"
		fcc "ECMD"
		fcc "ECRC"
		fcc "ESEQ"
		fcc "EADR"
		fcc "EPRM"

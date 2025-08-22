PAGE_MAP equ STACK ;right after stack

bmalloc_init
	ldx #PAGE_MAP
	ldd #0
	ldy #40 ;80 bytes for bitmap

1	std ,x++
	dey
	bne 1b
	
	;mark stack and page mapareas
	lda #$ff
	sta PAGE_MAP
	
	rts
	
;in: b: bank no
;	x: addr
	
bmfree
	cmpx #$8000
	bls 1f
	;not ran for LOWRAM addrs
	clra
	aslb
	rola
	aslb
	rola
	aslb
	rola
	aslb
	rola
	aslb
	rola
	aslb
	rola
	aslb
	rola ;mul by 64
	pshs d ; save upper bank bp
	
	tfr x, d
	tfr a, b;div by 256, convert into bp
	clra
	addd ,s++ ;add from saved upper bank bp and pop it
	pshs b ;preserve lower byte idx
	
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb ; div by 8
	
	tfr d,x 
	
	leax PAGE_MAP,x ;get byte idx
	puls b
	andb #0b111
	incb
	clra
	orcc #$1
3	rola
	decb
	bne 3b
	
	eora #$ff
	anda ,x
	sta ,x
	rts
	
1	tfr x,d
	
	tfr a,b
	pshs b
	clra 
	lsrb;/8 because bitmap
	lsrb
	lsrb
	tfr d,x
	
	leax PAGE_MAP,x ;get byte idx
	
	puls b
	andb #0b111 ;limit b range 
	incb
	clra
	orcc #$1 ;set carry
2	rola
	decb
	bne 2b
	
	eora #$ff;bitwise not
	
	anda ,x
	sta ,x
	rts
		
	
	
;in: B: number of pages
;out:
;X: address 
;b: bank no
;when fail: addr = 0, bank = 0
bmalloc
	pshs b
	ldx #PAGE_MAP
	
	lda #$1
1	bita ,x ;test loop
	bne 3f
	;runs if the block is free	
	decb
	beq 6f ;found 
	
	bra 4f
3	
	;runs if a used block is encountered
	ldb ,s ;load old pagesfrom stack

4	asla
	bne 1b ;conitnue iterating if mask isnt zero
	
	lda #$1
	inx
	cmpx #(PAGE_MAP+80)
	bne 1b
	
;fail
	leas 1,s ;pop b in stack
	ldx #$0
	clrb
	rts 
6	
	;a = mask, x = loc in pagemap
	;set the bit
	ora ,x
	sta ,x ;set the bit in x
	
	leax -PAGE_MAP, x
	;convert mask to 1-8 idx 
	clrb
7	incb
	lsra
	bne 7b  
	decb ;adjust to 0-7 idx
	pshs b
	tfr x,d 
	;mul d by 8
	aslb
	rola
	aslb
	rola
	aslb
	rola
	
	addb ,s+ ; add the previous value and pop it  after
	tfr d,x 
	
	puls b ;pop previous b 
	decb ; adjust
	negb ; make negative
	leax b,x ; get original block addr
	
	cmpx #$80 ;is it over lowram?
	beq 8f ;jmp if yes
	
	; if no then we just mul it 
	
	tfr x,d
	
	tfr b,a 
	clrb
	tfr d,x  ;mul by 256
	rts 
	
8	leax -$80,x  ;sub by 80 to get banked ram addr 
	tfr x,d
	lsra
	rora
	lsra
	rora
	lsra
	rora
	lsra
	rora
	lsra
	rora
	lsra
	rora
	;bank number in b
	pshs b
	tfr x,d
	tfr b,a
	clrb
	ora #$c0 ;get address 
	tfr d,x
	puls b 
	
	rts
	
	
	

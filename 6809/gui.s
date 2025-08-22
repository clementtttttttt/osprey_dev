gui_init:
	;clr scrn 
	ldx #0
	ldy #0
	jsr setc ;set cursor coord to 0,0 
	
	ldx #2400
	
1	pshs x
	lda #' '
	jsr putchar
	puls x
	dex
	bne 1b
	
	ldx #0
	ldy #0
	jsr setc
	
	rts
		




;x: addr of 1 block sz 
;gui_queue class:
;
;	short queue_len
;	short curr_sel (FFFF=no sel)
;	gui_widget* widgets[] 
;x = ptr to mem block
;y= queue_len
gui_init_queue:
	sty ,x++ ;inc to widgets
	ldd #$ffff
	std ,x++
	ldd #$0
1	std ,x++
	dey
	bne 1b
	rts

;x = ptr to queue
;y = widget ptr
gui_queue_add_widget:
	leax 4,x
1	ldd ,x++  
	bne 1b ;used, continue
	sty ,--x
	rts
	

;gui_callback_pair
;char name[14]
;void *callback


;gui_menu
;char name[12]
;short len
;short curr_sel (FFFF = no sel)
;gui_callback_pair[]
;x = ptr to mem block
;y = pointer to name
;s+2 = len
gui_menu_constructor:
	lda #12
1	ldb ,y+
	stb ,x+
	beq 2f ;zero terminate
	deca
	bne 1b
2	ldy 2,s ;store len
	sty ,x++
	ldd #$FFFF
	std ,x++
	exg y,d
	aslb 
	rola
	aslb
	rola
	aslb
	rola ;mul by 8
	exg y,d
	ldd #$0
3	std ,x++ ;zero out len
	dey
	bne 3b
	rts

	
	


;x = ptr to gui_menu
;s+2: name
;s+4: callback ptr 
gui_menu_add_item:
	leax 16,x
1	leax 14,x
	ldx ,x++
	beq 1b ;empty entry
	leax -16, x
	ldy 2,s ;get name ptr
	lda #14
2	ldb ,y+
	stb ,x+
	beq 3f
	deca
	bne 1b
3	ldy 4,s
	sty ,x ;store callback ptr
	rts
	
;x = ptr to gui_menu
;s+2 = x coord
;s+4 = y coord
gui_menu_draw:
	pshs x
	ldx 2,s
	ldy 4,s
	jsr setc
	puls x
	jsr putchar
	
	
	
	rts

;gui_toolbar class:
; uint8_t	type = 0
; uint16_t 	x
; uint16_t 	y
; uint16_t num_menus
; uint16_t x_sz
; uint16_t curr_sel_menu (FFFF=no sel)
; gui_menu*[]
;INPUTS: 
; x: addr for class
; s+2: x
; s+4: y
; s+6: num_menus
; s+8: x_sz
gui_toolbar_constructor
	lda #0
	sta ,x+
	ldy 2,s
	sty ,x++
	ldy 4,s
	sty ,x++
	ldy 6,s
	sty ,x++
	ldy 8,s
	sty ,x++
	ldd #$ffff ;curr_sel_menu
	std ,x++
	ldd #0
1	std ,x++
	dey
	bne 1b
	rts
	
	
;toolkar ptr at y
gui_toolbar_draw:
	pshs y
	
	ldx 1,y
	ldy 3,y
	jsr setc
	
	lda #0b111001 ; white on bliue
	jsr setfgbg
	
	puls y
	
	ldx 7,y ;y already advanced by one
	
	pshs x,y
	;print bar
1	lda #205 ;double line horizontal
	jsr putchar
	dex
	bne 1b
	
	puls x,y
	
	pshs y ;toolbar addr in s+4
	ldx 3,x ;y coord
	pshs x ; s+2 = y coord
	ldx 1,x ;x coord in s
	pshs x ; s = x coord
	
	ldy 4,s ;get toolbar addr
	leay 11, y ; advance to list of menus
	
2	ldx ,y++ ;x holds menu ptr 
	beq 3f
	pshs y
	jsr gui_menu_draw
	puls y
	bra 2b
3
	
	
	leas 6,s ;clean up stack

	rts
;x = pointer to gui_toolbar
;y = gui_menu*

gui_toolbar_add_menu:
	leax 9,x
1	ldd ,x++
	bne 1b
	leax -2, x
	sty ,x
	rts

;x = queue
gui_draw_queue:
	leax 4,x

1	ldy ,x++
	beq 2f
	;y now holds pointer to a widget
	ldb ,y ; b holds type of widget
	pshs x
	clra
	aslb
	rola ; mul by 2 for idx
	
	ldx #gui_draw_func_table
	;y = widget addr 
	jsr [d,x] ;call func ptr + idx
	
	puls x
	bra 1b
2
	
	rts
	
gui_draw_func_table
	fdb gui_toolbar_draw
	




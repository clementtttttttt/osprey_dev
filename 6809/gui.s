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
;	short queue_len
;	char* name
;	short x
;	short y
;	short xsz
;	short ysz
;	short curr_sel (FFFF=no sel)
;	gui_widget* widgets[] 
;x = ptr to mem block
;y= queue_len
;s+2 = name ptr
;s+4 = x
;s+6 = y
;s+8 = xsz
;s+10 = ysz
gui_init_queue:
	sty ,x++ ;inc to widgets
	ldd 2,s
	std ,x++
	ldd 4,s
	std ,x++
	ldd 6,s
	std ,x++
	ldd 8,s
	std ,x++
	ldd 10,s
	std ,x++
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
	leax 14,x
1	ldd ,x++  
	bne 1b ;used, continue
	sty ,--x
	rts
	



;gui_menu
;char *name
;short len
;short curr_sel (FFFF = no sel)
;void * on_item_clicked
;char* names[]
;x = ptr to mem block
;y = pointer to name
;s+2 = len
;s+4 = on_item_clicked
gui_menu_constructor:
	sty ,x++
	ldy 2,s ;store len
	sty ,x++
	ldd #$FFFF
	std ,x++
	ldy 4,s
	std ,x++

;setting names*[] arr
	ldd #$0
3	std ,x++ ;zero out len
	dey
	bne 3b
	rts

	
;gui_list_constructor
;uint8_t type = 1
;uint16_t x
;uint16_t y
;uint16_t curr_sel (FFFF=no sel)
;uint16_t num_columns
;uint16_t num_rows
;void* on_click
;char *column_names[num_columns]
;char* rows_data[num_rows]
;
;each pointer in rows_data points to r1\0r2\0r3\0
;

;x: address for class
;s+2 = x local
;s+4 = y local
;s+6 = num_columns
;s+8 = num_rows
;s+10 = on_click_handler
gui_column_list_constructor:
	lda #1 ;gui_list type
	sta ,x+
	ldy 2,s
	sty ,x++
	ldy 4,s
	sty ,x++
	ldy #$ffff
	sty ,x++
	ldy 6,s ;numcol
	sty ,x++
	ldy 8,s  ;numrow
	sty ,x++
	ldy 10,s ;on_click handler
	sty ,x++
	
	
	ldy 6,s
	ldd #0
	;y = num of columns
1	std ,x++
	dey 
	bne 1b
	
	ldy 8,s 
	;y = num_of_rows
2	std ,x++
	dey
	bne 2b
	
	rts
	 
;gui_list_add_row:

;x: address for class
;
	

	



;x = ptr to gui_menu
;s+2: name ptr
gui_menu_add_item:
	leax 8,x
1	ldx ,x++
	bne 1b ;empty entry
	
	leax -2, x
	ldy 2,s ;get name ptr
	sty ,x++
	rts
	
;x = ptr to gui_menu
;s+2 = x coord
;s+4 = y coord
; MUST PRESERVE Y
gui_menu_draw:
	pshs x,y
	ldx 6,s
	ldy 8,s
	jsr setc
	puls x,y
	ldx ,x
	jsr putstr
	
	
	
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
;s+2 = x
;s+4 = y
gui_toolbar_draw:
	pshs y
	
	ldx 1,y
	ldd 4,s
	leax d,x
	ldy 3,y
	ldd 6,s
	leay d,y
	jsr setc
	
	
	
	puls y
	
	ldx 7,y ;load x width
	
	pshs x,y
	;print bar
1	lda #205 ;double line horizontal
	jsr putchar
	dex
	bne 1b
	
	puls x,y
	
	leas -6, s
	sty 4,s ;toolbar addr in s+4
	ldx 3,y ;toolbar y coord
	ldd 10,s ;load y offset
	leax d,x ;add y offset
	stx 2,s ; s+2 = y coord
	ldx 1,y ;toolbar x coord in s    
	inx ;menu coord 
	ldd 8,s ;load x offset 
	leax d,x ;add x offset 
	stx ,s ; s = x coord
	
	ldy 4,s ;get toolbar addr
	leay 11, y ; advance to list of menus
	
2	ldx ,y++ ;x holds menu ptr 
	beq 3f
	jsr gui_menu_draw
	
	ldx ,s; margin of two between menu items
	leax 2,x
	stx ,s
	
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
	
	lda #0b111001
	jsr setfgbg
	
	leas -10, s
	ldd 4,x
	std ,s ;x
	ldd 6,x
	std 2,s ;y
	ldd 8,x
	std 4,s ;xsz
	ldd 10,x
	std 6,s ;ysz
	
	pshs x
	
	ldx 2,s
	ldy 4,s
	jsr setc
	
	ldx ,s
	ldx 2,x ;load name ptr
	jsr putstr
	
	puls x
	
	ldy 2,s ;load y
	iny
	
	sty 2,s ;widgets at next line
	
	leax 14, x ;skip to list of widgets
	

1	ldy ,x++
	beq 2f
	;y now holds pointer to a widget
	ldb ,y ; b holds type of widget
	clra
	aslb
	rola ; mul by 2 for idx
	
	 
	
	stx 8, s; backup x

	ldx #gui_draw_func_table
	;y = widget addr 
	jsr [d,x] ;call func ptr + idx
	
	ldx 8,s ;restore x

	bra 1b
2

	leas 10,s 
	
	
	rts
	
gui_draw_func_table
	fdb gui_toolbar_draw
	




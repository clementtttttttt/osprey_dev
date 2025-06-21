;a = part id, y = sector 0 image
;a:b:x = lba
mbr_read_part_lba
	leay $1be,y ;add y by part ent offset
	asla 
	asla
	asla
	asla ;multiply by 16
	adda #8 ;part ent lba offset
	
	leay a, y ;add offset to y
	
	ldd ,y++ ;0:16
	exg a,b	;little endian bullshot
	tfr d,x ;transfer 16:0 to x
	ldd ,y++ ;16:32
	exg a,b ;more little endian bullshot
	
	rts

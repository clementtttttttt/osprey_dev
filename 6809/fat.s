;x = 16bit op
;a = 8 bit op

;a:b:x = 32 bit output
mul_16_8
	pshs a
	tfr x, d			;higher bits at a
	ldb ,s ;a from stack now at b
	mul
	
	leas -1,s ;bit shift result by 8
	pshs d ;push it on stack for now 
	lda #0
	sta 2,s ;zero bit shift byte
	
	
	tfr x,d ;8:0 at b
	lda 3,s ;get previous a value, again
	mul
	
	addd 1,s ;add with 16:0 of last mul
	tfr d,x ;lower bits now at x
	
	puls b ;24:16 now at b 
	adcb #0 ; add carry flag
	lda #0 ;empty 32:16 
	
	leas 3, s ; free everything 
	rts 
	
	
	
	

;y = fat superblock image
;in = a:b:x = partition lba
;a:b:x = root lba
fat_get_root_lba
	pshs d
	pshs x
	ldd $E,y ;reserved sectors amount
	exg a,b ;convert to big endian
	pshs d; save rsvd sects on stack
	
	ldd $16, y ;sects per fat
	exg a,b ;big endian
	tfr d, x ;mul op 1
	
	lda $10, y ;fat counts
	
	bsr mul_16_8
	
	exg x,d ;swap 32:16 and 16:0
	
	addd ,s++ ;free rsvd_sects,add rsvd_sects 16:0 and sectss_per_fats*count
	exg x,d	;swsapped back
	adcb #0 ;add dcarry to upper bits
	
	
	exg x,d ;16:0 now at d
	
	addd ,s++ ;add ANS and part lba, and free it
	exg x,d; 32:16 now at d,16:0 now at x
	adcb ,s+ ;add 24:16
	adca ,s+ ;add 32:24
	
	
	rts ;we're done, results stored 
	
;*sp = pointer tot a dirents[];
;d = idx
;d = ccluster low 16
fat_get_cluster_from_dirent
	aslb ;mul by 32 (dirent size)
	rola
	aslb
	rola 
	aslb
	rola
	aslb
	rola
	aslb
	rola
	
	addd ,s++ ;add by dirents[] ptr and pop it
	
	tfr d,x ;transfer the address to x
	
	ldd 26,x ;load low16 bits of cluster number
	exg a,b ;convert to big endian
	rts

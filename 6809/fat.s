;FAT_INFO adt

FI_FIRST_FAT_SECTOR equ 0
FI_ROOT_LBA equ 2



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
	

;in: d = cluster number
;	y = buff;
;   a:b:x = 
fat_read_from_cluster:


;y = fat superblock image
;in = a:b:x = partition lba
;a:b:x = root directory entries lba
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
	

DIRENT_CLUSTER_NUMBER_OFF equ 26


;d = cluster number
;0,s = first fat sector (32 bit lba)
;clobbers x & y

;d = next_cluster
;x = ffff if error else 0
fat_get_next_cluster:
	pshs u
	tfr s, u
	leas -512, s ;allocate space on stack
	
	aslb
	rola ;mul active cluster by two to get offset in fat table
	
	pshs d
	
	;TODO: sector size other than 512
	
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb ;div by 512 sector size
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb
	lsra
	rorb

	addd 4,s
	tfr d,x ;low 16 bits into x
	
	ldd #$0 ;upper 32 now 0
	adcb 5,s ;upper 24-16 bits of first fat sector added
	adca 6,s ;upper 32-24 bits of first fat scector added

	;a:b:x now contains sector number for fat table
	;y = read buffer addr
	
	leay -512, u;read buffer addr
	
	jsr sd_read_sect
	
	cmpa #$fe 
	bne 1f ;error
	
	puls d
	
	;now -512, u contains fact table cluster
	andb #0b111111111 ;modulo with AND
	leay -512, u
	
	ldd b,y ;read from buffer with offset
	
	ldx #$0
	
	tfr u,s ;restore original stack pointer
	puls u ;restore original frame pointer
	
	rts

1	ldx #$ffff ;set x to error state
	tfr u,s
	puls u
	rts
	
	



;*sp = pointer tot a dirents[];
;d = idx
;return value:
;d = cluster 16 bit number (we're using fat 16)
fat_get_cluster_from_dirent
	aslb ;mul idx by 32 (dirent size)
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
	
	ldd DIRENT_CLUSTER_NUMBER_OFF,x ;load low16 bits of cluster number
	exg a,b ;convert to big endian
	rts

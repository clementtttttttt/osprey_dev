;interrupt handler
;called for hardware interrupts 

irq_handler:
	lda VIA1_IFR
	bita #$40;did T1 trigger (watchdog timer tick)
	bne 1f 
	;bra 1f

	
	
	rti
1	;watchdog timeout handling

	lda WATCHDOG_REMAINING_TICKS
	;if 0 then terminate process
	beq 2f
	
	;if not then decrement and do other stuff
	deca 
	sta WATCHDOG_REMAINING_TICKS 
	
	
		ldx #gui_str
	jsr putstr
	
	lda #$40
	sta VIA1_IFR ;reset interrupt
	

	rti 

2	;onWatchdogExpired

	;reset the timer
	lda #$ff
	sta WATCHDOG_REMAINING_TICKS
	
	rti

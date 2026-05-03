#include <VIA6522.h>

VIA6522::VIA6522(){
	t1count = 0;
	t2count = 0;
	t1latch = 0;
	t2latch = 0;
	sr = 0;
	
	reset();
}

void VIA6522::reset(){
	portb=0;
	porta=0;
	ddrb=0;
	ddra=0;
	
	acr=0;
	pcr=0;
	ifr=0;
	ier=0;
}

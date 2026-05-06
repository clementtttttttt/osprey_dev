#include <VIA6522.h>
#include <iostream>
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

uint8_t VIA6522::ext_get_sr(){
	return sr;
}
uint8_t VIA6522::reg_read(uint16_t in){
		REG6522_R addr = static_cast<REG6522_R>(in);
		std::cout << "reg read " << std::hex << in<<std::endl;
		switch(addr){

			case IFRR:
			{
				//ifr clears when bit is 1, except bit 7
				uint8_t ret = ifr;
				if(ret & 0x7f){
					ret |= 0x80; //ifr bit 7 is set if any of the other bits are set
				} 
				else{
					ret &= 0x80;
				}
				
				return ifr;
				break;
			}
			case SRR:
				return sr;
				break;
				
			default:
				std::cout <<"unimplemented VIA reg " << std::hex<<in << std::endl;
				break;
		}
		return 0xff;
}

void VIA6522::ext_write_portb(uint8_t active_bits, uint8_t data){
	if(active_bits & ddrb){
		std::cout << "WARNING: BUS CONFLICT ON PORTB!" << std::endl;
	}
	
	portb &= active_bits;//unset data in input bits
	data &= ~(active_bits); //unset data in unused bits 
	
	portb |= data; //combine data
	
	return;
}

void VIA6522::ext_write_porta(uint8_t active_bits, uint8_t data){
	if(active_bits & ddra){
		std::cout << "WARNING: BUS CONFLICT ON PORTA!" << std::endl;
	}
	
	porta &= active_bits;//unset data in input bits
	data &= ~(active_bits); //unset data in unused bits 
	
	porta |= data; //combine data
	
	return;
}

void VIA6522::reg_write(uint16_t in, uint8_t data){
	REG6522_W addr = static_cast<REG6522_W>(in);
	switch(addr){
		case ORAW:
			porta = data;
			ifr &= ~(IFR_CA1); //clear ca1 int
			break;
		case ORBW:
			portb = data;
			ifr &= ~(IFR_CB1); //clear ca1 int
			break;
		case DDRBW:
			ddrb = data;
			break;
		case DDRAW:
			ddra = data;
			break;
		case IFRW:
			//ifr clears when bit is 1, except bit 7
			data = ~data;
			ifr &= data;
			break;
			
		case SRW:
			sr = data;
			ifr |= 0b100; //IFR SR data transfer complete bit, TODO: accurate timing of spi
			break;
		default:
			std::cout <<"unimplemented VIA reg " << std::hex <<in << std::endl;
			break;
		
		
	}
	
}

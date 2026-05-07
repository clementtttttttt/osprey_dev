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
		switch(addr){
			case IRAR: 
				return porta;
				break;
			case IRBR:
				return portb;
				break;
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
				//std::cout <<"unimplemented VIA reg " << std::hex<<in << std::endl;
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

void VIA6522::ext_set_ca1(){
	ifr |= IFR_CA1;
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

void VIA6522::on_cb2_w(bool st){
	if(cb2_w_cb != nullptr){ 
		cb2_w_cb(st);
	}
	
}
void VIA6522::on_ca2_w(bool st){

	if(ca2_w_cb != nullptr){ 
		ca2_w_cb(st);
	}
}

void VIA6522::set_cb2_w_cb(void(*in)(bool)){
	cb2_w_cb = in;
	
}
void VIA6522::set_ca2_w_cb(void(*in)(bool)){
	ca2_w_cb = in;

}

void VIA6522::set_pb_w_cb(void(*in)(uint8_t)){
	pb_w_cb = in;

}

void VIA6522::on_pb_w(uint8_t in){
	if(pb_w_cb != nullptr){
		pb_w_cb(in);
	}
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
			on_pb_w(data);
			break;
		case DDRBW:
			ddrb = data;
			break;
		case DDRAW:
			ddra = data;
			break;
		case PCRW:
		{
			pcr = data;
			
			PCR_CX_MODES b_bits = static_cast<PCR_CX_MODES>((pcr >> 5) & 0x7);
			PCR_CX_MODES a_bits = static_cast<PCR_CX_MODES>((pcr >> 1) & 0x7);
			switch(b_bits){
				case CX2_HIGH:
					on_cb2_w(true);
					break;
				case CX2_LOW:
					on_cb2_w(false);
					break;
				default:
					//std::cout << "unimplemented PCR write CB " << b_bits << std::endl;
					break;
			}
			
			switch(a_bits){
				case CX2_HIGH:
					on_ca2_w(true);
					break;
				case CX2_LOW:
					on_ca2_w(false);
					break;
				default:
				//	std::cout << "unimplemented PCR write Ca " << a_bits <<  std::endl;
					break;
			}
		}
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
		//	std::cout <<"unimplemented VIA reg " << std::hex <<in << std::endl;
			break;
		
		
	}
	
}

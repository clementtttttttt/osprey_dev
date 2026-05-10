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
	
	acr.raw=0;
	pcr=0;
	ifr=0;
	ier=0;
	ca2 = false;
	ca1 = true;
	cb2 = false;
	cb1 = false;
	ca2_low_counter = 0;
	
}

uint8_t VIA6522::ext_get_sr(){
	return sr;
}
uint8_t VIA6522::reg_read(uint16_t in){
		REG6522_R addr = static_cast<REG6522_R>(in);
		switch(addr){
			case T1_COUNT_LR:
				return t1count & 0xff;
			case T1_COUNT_HR:
				return (t1count >> 8) & 0xff;
			case T1_LATCH_LR:
				return t1latch & 0xff;
			case T1_LATCH_HR:
				return (t1latch >> 8) & 0xff;
			case T2_COUNT_LR:
				return t2count & 0xff;
			case T2_COUNT_HR:
				return (t2count >> 8) & 0xff;
			case DDRBR:
				return ddrb;
			case DDRAR:
				return ddra;
			case IRAR: 
				ifr &= ~(IFR_CA1 | IFR_CA2);
				
				if(((pcr >> 1) & 0b111) == 0b101){
					ca2_low_counter = 2;
					on_ca2_w(false);
				} 
				return porta;
				break;
			case IRA_NOHSR:
				return porta;
				break;
			case IRBR:
				ifr &= ~IFR_CB1;
				return portb;
				break;
			case IERR:
				return ier;
				break;
			case IFRR:
			{
				uint8_t ret = ifr;
				if(ret & 0x7f){
					ret |= 0x80;
				} 
				else{
					ret &= 0x7f;
				}
				
				return ifr;
				break;
			}
			case SRR:
				return sr;
				break;
			case ACRR:
				return acr.raw;
				break;
			case PCRR:
				return pcr;
				break;


		}
									#ifdef VIA_DBG

				std::cout <<"unimplemented VIA reg " << std::hex<<in << std::endl;
								#endif

		return 0xff;
}

void VIA6522::ext_write_portb(uint8_t active_bits, uint8_t data){
	portb = (portb & ~active_bits) | (data & active_bits);
}

void VIA6522::ext_set_ca1(uint32_t v){
	if(pcr & 1){
		if(ca1 == false && v){
			fire_irq(IFR_CA1);
		}
	}
	else{
		if(ca1 == true && !v){
			fire_irq(IFR_CA1);
		}
	}
	ca1 = (v != 0);
}


void VIA6522::ext_write_porta(uint8_t active_bits, uint8_t data){
	porta = (porta & ~active_bits) | (data & active_bits);
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

void VIA6522::set_on_irq_cb(void(*in)()){
	on_irq_cb = in;
}

void VIA6522::on_pb_w(uint8_t in){
	if(pb_w_cb != nullptr){
		pb_w_cb(in);
	}

	}

void VIA6522::fire_irq(uint8_t mode){
	ifr |= mode;
	
	if(on_irq_cb != nullptr && (mode & ier)){
		on_irq_cb();
	}
}

void VIA6522::phi2_tick(){
	switch(static_cast<ACR_T1_MODES>(acr.t1_modes)){
		case T1_FR_PB7:
		case T1_OS:
			if(t1count > 0){
				if(--t1count ==0){
					fire_irq(IFR_T1); //set ifr
				}
				
			}
		break;
		case T1_FR:
		{
			if(t1count>0){	
				--t1count;
			}
			else{
				ifr |= 1 << 6;
				fire_irq(IFR_T1);
				t1count = t1latch;
			}
				
		}
		
	}
	
	if(ca2_low_counter){
		--ca2_low_counter;
		if(ca2_low_counter == 0){
			on_ca2_w(true);
		}
	}
	
}

void VIA6522::reg_write(uint16_t in, uint8_t data){
	REG6522_W addr = static_cast<REG6522_W>(in);
	switch(addr){
		case ORA_NOHSW:
			porta = data;
			break;
		case ORAW:
			porta = data;
			ifr &= ~(IFR_CA1 | IFR_CA2); //clear ca1 int
			
			if(((pcr >> 1) & 0b111) == 0b101){
				ca2_low_counter = 2;
				on_ca2_w(false);
			} 
			
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
		case T1_LATCH_LW:
			t1latch &= 0xff00; //clear low
			t1latch |= data;
			break;
		case T1_LATCH_HW:
			t1latch &= 0xff; //clear hi
			t1latch |= ((uint16_t)data<<8);
			break;
		case T1_COUNT_HW:
			t1count &= 0xff;
			t1count |= (uint16_t)data << 8;
			break;
		case T2_COUNT_HW:
			t2count = 0;
			t2count = (uint16_t)data << 8;
			t2count |= t2latch & 0xff;
			ifr &= ~(IFR_T2);
			break;
		case T2_LATCH_LW:
			t2latch &= 0xff00;
			t2latch |= data;
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
				#ifdef VIA_DBG
					std::cout << "unimplemented PCR write CB " << b_bits << std::endl;
									#endif

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
				#ifdef VIA_DBG
					std::cout << "unimplemented PCR write Ca " << a_bits <<  std::endl;
				#endif
					break;
			}
		}
			break;
		case IFRW:
			//ifr clears when bit is 1, except bit 7
			data = ~data;
			ifr &= data;
			break;
		case IERW:
			ier = data;
			break;
		case SRW:
			sr = data;
			ifr |= 0b100; //IFR SR data transfer complete bit, TODO: accurate timing of spi
			break;
		case ACRW:
			acr.raw = data;
			break;

		
		
	}
									#ifdef VIA_DBG

			std::cout <<"unimplemented VIA reg write " << std::hex <<in << std::endl;
							#endif

}

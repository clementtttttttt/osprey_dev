#ifndef VIA6522_H
#define VIA6522_H


#include <cstdint>
class VIA6522{
	
	uint8_t portb;
	uint8_t porta;
	uint8_t ddrb;
	uint8_t ddra;
	
	uint16_t t1count;
	uint16_t t1latch;
	uint16_t t2count;
	uint16_t t2latch;
	uint8_t sr;
	
	uint8_t acr;
	uint8_t pcr;
	uint8_t ifr;
	uint8_t ier;
	
	enum IFR_BITS{
		IFR_CA2 = 0b1,
		IFR_CA1 = 0b10,
		IFR_SR  = 0b100,
		IFR_CB2 = 8,
		IFR_CB1 = 16,
		IFR_T2 = 32,
		IFR_T1 = 64
	};
	

	enum REG6522_R{
			IRBR,
			IRAR,
			DDRBR,
			DDRAR,
			T1_COUNT_LR,
			T1_COUNT_HR,
			T1_LATCH_LR,
			T1_LATCH_HR,
			T2_COUNT_LR,
			T2_COUNT_HR,
			SRR,
			ACRR,
			PCRR,
			IFRR,
			IERR,
			IRA_NOHSR
		
	};
	
		enum REG6522_W{
			ORBW,
			ORAW,
			DDRBW,
			DDRAW,
			T1_LATCH_LW,
			T1_COUNT_HW,
			T1_LATCH_L2W,
			T1_LATCH_HW,
			T2_LATCH_LW,
			T2_COUNT_HW,
			SRW,
			ACRW,
			PCRW,
			IFRW,
			IERW,
			ORA_NOHSW
		
	};
	
public:
	void reg_write(uint16_t addr, uint8_t data);
	VIA6522();
	uint8_t reg_read(uint16_t addr);
	void ext_write_portb(uint8_t active_bits, uint8_t data);
	void ext_write_porta(uint8_t active_bits, uint8_t data);
	
	void ext_set_ca1();
	
	void reset();
	
	uint8_t ext_get_sr();
};

#endif

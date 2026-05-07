#ifndef VIA6522_H
#define VIA6522_H

#include <simavr/sim_avr.h>

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
	
	enum PCR_CX_MODES{
		CX2_NEG_SF_RW_CLR = 0,
		CX2_NEG_SF_RW_NOCLR,
		CX2_POS_SF_RW_CLR,
		CX2_POS_SF_RW_NOCLR,
		CX2_LOW_ON_RW_HIGH_ON_CX1,
		CX2_LOW_PULSE_ON_RW,
		CX2_LOW,
		CX2_HIGH
		
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
	
	bool cb2;
	bool ca2;
	void (*cb2_w_cb)(bool st);
	void (*ca2_w_cb)(bool st);

	void (*pb_w_cb)(uint8_t st);

public:
	void reg_write(uint16_t addr, uint8_t data);
	VIA6522();
	uint8_t reg_read(uint16_t addr);
	void ext_write_portb(uint8_t active_bits, uint8_t data);
	void ext_write_porta(uint8_t active_bits, uint8_t data);
	
	void ext_set_ca1();
	
	void reset();
	
	uint8_t ext_get_sr();
	
	void on_cb2_w(bool st);
	void on_ca2_w(bool st);
	
	void on_pb_w(uint8_t in);
	
	void set_cb2_w_cb(void(*)(bool));
	void set_ca2_w_cb(void(*)(bool));

	void set_pb_w_cb(void(*)(uint8_t));

};

#endif

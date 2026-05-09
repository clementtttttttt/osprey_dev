#ifndef PS2KEYBOARD_H
#define PS2KEYBOARD_H

#include <cstdint>
#include <queue>
#include <SDL.h>
#include <simavr/sim_avr.h>

class PS2Keyboard {
public:
	PS2Keyboard();

	void handle_event(const SDL_Event &ev);
	void tick(avr_t *avr);

private:
	enum { PS2_CLK_HALF_PERIOD = 83 };

	void queue_scan(uint8_t code);
	void queue_scan_ext(uint8_t code);

	std::queue<uint8_t> m_tx;

	uint16_t m_frame;
	uint8_t m_frame_bits;
	uint8_t m_frame_idx;
	int m_clk_counter;
	bool m_clk_state;
	bool m_data_state;
	bool m_sending;
	bool m_idle;
	bool m_clock_pending;
	uint16_t m_inter_frame_delay;

};

#endif

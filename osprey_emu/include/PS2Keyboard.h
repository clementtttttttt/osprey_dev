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
	enum { PS2_CLK_HALF_PERIOD = 30 };

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

	static const uint8_t sdl_to_ps2[SDL_NUM_SCANCODES];
};

#endif

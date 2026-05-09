#include "PS2Keyboard.h"
#include <simavr/sim_avr.h>
#include <simavr/avr_ioport.h>
#include <iostream>
#include <iomanip>

//#define PS2_DEBUG
#ifdef PS2_DEBUG
#define PS2_LOG(x) std::cerr << "[PS2] " << x << std::endl
#else
#define PS2_LOG(x)
#endif

const uint8_t PS2Keyboard::sdl_to_ps2[SDL_NUM_SCANCODES] = {
	//  0-7
	0, 0, 0, 0, 0x1C, 0x32, 0x21, 0x23,
	//  8-15
	0x24, 0x2B, 0x34, 0x33, 0x43, 0x3B, 0x42, 0x4B,
	// 16-23
	0x3A, 0x31, 0x44, 0x4D, 0x15, 0x2D, 0x1B, 0x2C,
	// 24-31
	0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A, 0x16, 0x1E,
	// 32-39
	0x26, 0x25, 0x2E, 0x36, 0x3D, 0x3E, 0x46, 0x45,
	// 40-47
	0x5A, 0x76, 0x66, 0x0D, 0x29, 0x4E, 0x55, 0x54,
	// 48-55
	0x5B, 0x5D, 0x5D, 0x4C, 0x52, 0x0E, 0x41, 0x49,
	// 56-63
	0x4A, 0x58, 0x05, 0x06, 0x04, 0x0C, 0x03, 0x0B,
	// 64-71
	0x83, 0x0A, 0x01, 0x09, 0x78, 0x07, 0, 0x7E,
	// 72-79
	0, 0x70, 0x6C, 0x7D, 0x71, 0x69, 0x7A, 0x74,
	// 80-87
	0x6B, 0x72, 0x75, 0x77, 0x4A, 0x7C, 0x7B, 0x79,
	// 88-95
	0x5A, 0x69, 0x72, 0x7A, 0x6B, 0x73, 0x74, 0x6C,
	// 96-103
	0x75, 0x7D, 0x70, 0x71, 0x61, 0x2F, 0, 0,
	// 104-223 (all zeros)
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 224-231
	0x14, 0x12, 0x11, 0x1F, 0x14, 0x59, 0x11, 0x27,
};

PS2Keyboard::PS2Keyboard()
	: m_frame(0)
	, m_frame_bits(0)
	, m_frame_idx(0)
	, m_clk_counter(0)
	, m_clk_state(true)
	, m_data_state(true)
	, m_sending(false)
{
}

void PS2Keyboard::queue_scan(uint8_t code)
{
	PS2_LOG("queue 0x" << std::hex << (int)code << std::dec << " (tx=" << m_tx.size() << ")");
	m_tx.push(code);
}

void PS2Keyboard::queue_scan_ext(uint8_t code)
{
	PS2_LOG("queue ext 0xE0 0x" << std::hex << (int)code << std::dec);
	m_tx.push(0xE0);
	m_tx.push(code);
}

void PS2Keyboard::handle_event(const SDL_Event &ev)
{
	if (ev.type != SDL_KEYDOWN && ev.type != SDL_KEYUP) return;

	SDL_Scancode sc = ev.key.keysym.scancode;
	if (sc >= SDL_NUM_SCANCODES) return;

	uint8_t code = sdl_to_ps2[sc];
	if (!code) {
		PS2_LOG("unmapped scancode " << (int)sc);
		return;
	}

	PS2_LOG("SDL sc=" << (int)sc << " -> ps2=0x" << std::hex << (int)code << std::dec << (ev.type == SDL_KEYUP ? " UP" : " DOWN"));

	bool extended = (sc == SDL_SCANCODE_KP_ENTER || sc == SDL_SCANCODE_KP_DIVIDE || sc == SDL_SCANCODE_LGUI || sc == SDL_SCANCODE_RCTRL || sc == SDL_SCANCODE_RALT || sc == SDL_SCANCODE_RGUI || sc == SDL_SCANCODE_APPLICATION || sc == SDL_SCANCODE_INSERT || sc == SDL_SCANCODE_HOME || sc == SDL_SCANCODE_PAGEUP || sc == SDL_SCANCODE_DELETE || sc == SDL_SCANCODE_END || sc == SDL_SCANCODE_PAGEDOWN || sc == SDL_SCANCODE_RIGHT || sc == SDL_SCANCODE_LEFT || sc == SDL_SCANCODE_DOWN || sc == SDL_SCANCODE_UP);

	if (ev.type == SDL_KEYUP) {
		if (extended) {
			queue_scan_ext(0xF0);
			queue_scan(code);
		} else {
			queue_scan(0xF0);
			queue_scan(code);
		}
	} else {
		if (extended)
			queue_scan_ext(code);
		else
			queue_scan(code);
	}
}

static void ps2_set_pin(avr_t *avr, char port, int bit, bool val)
{
	avr_raise_irq(
		avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ(port), bit),
		val);

	avr_ioport_external_t ext;
	ext.name = port;
	ext.mask = (uint8_t)(1 << bit);
	ext.value = val ? (uint8_t)(1 << bit) : 0;
	avr_ioctl(avr, AVR_IOCTL_IOPORT_SET_EXTERNAL(port), &ext);
}

void PS2Keyboard::tick(avr_t *avr)
{
	if (!m_sending) {
		if (m_tx.empty()) return;

		uint8_t byte = m_tx.front();
		m_tx.pop();

		uint8_t parity = 1;
		for (int i = 0; i < 8; i++) parity ^= (byte >> i) & 1;

		m_frame = ((uint16_t)byte << 1) | ((uint16_t)parity << 9) | (1u << 10);
		m_frame_bits = 11;
		m_frame_idx = 0;
		m_sending = true;
		m_clk_counter = PS2_CLK_HALF_PERIOD;
		m_clk_state = true;
		m_data_state = true;

		PS2_LOG("TX byte 0x" << std::hex << (int)byte << std::dec << " parity=" << (int)parity);
	}

	m_clk_counter--;

	if (m_clk_counter > 0) return;

	m_clk_counter = PS2_CLK_HALF_PERIOD;

	if (m_clk_state) {
		m_clk_state = false;

		if (m_frame_idx == 0) {
			PS2_LOG("  start bit");
			ps2_set_pin(avr, 'B', 1, false);
			ps2_set_pin(avr, 'D', 2, true);
			m_frame_idx = 1;
		} else {
			PS2_LOG("  clock rising edge, sample bit=" << (int)(m_frame_idx - 1));
			ps2_set_pin(avr, 'D', 2, true);
			m_frame_idx++;

			if (m_frame_idx >= m_frame_bits) {
				m_sending = false;
				m_clk_state = true;
				m_data_state = true;

				PS2_LOG("TX complete, idle");

				ps2_set_pin(avr, 'D', 2, true);
				ps2_set_pin(avr, 'B', 1, true);
			}
		}
	} else {
		m_clk_state = true;
		m_data_state = (m_frame >> m_frame_idx) & 1;

		PS2_LOG("  data=" << (int)m_data_state << " bit=" << (int)m_frame_idx);

		ps2_set_pin(avr, 'B', 1, m_data_state);
		ps2_set_pin(avr, 'D', 2, false);
	}
}

#include "PS2Keyboard.h"
#include <simavr/sim_avr.h>
#include <simavr/avr_ioport.h>
#include <iostream>
#include <iomanip>

///#define PS2_DEBUG
#ifdef PS2_DEBUG
#define PS2_LOG(x) std::cerr << "[PS2] " << x << std::endl
#else
#define PS2_LOG(x)
#endif

const uint8_t PS2Keyboard::sdl_to_ps2[SDL_NUM_SCANCODES] = {
	0, 0x76,
	0, 0x05, 0x06, 0x04, 0x0C, 0x03, 0x0B, 0x83,
	0x0A, 0x01, 0x09, 0x78, 0x07, 0x7C, 0x7E, 0x7D,
	0, 0x45, 0x16, 0x1E, 0x26, 0x25, 0x2E, 0x36,
	0x3D, 0x3E, 0x46, 0, 0x66, 0x0D, 0x15, 0x58,
	0x4C, 0x24, 0x2D, 0x2C, 0x35, 0x3C, 0x43, 0x44,
	0x4D, 0x54, 0x5B, 0x5D, 0x4B, 0x42, 0x34, 0x33,
	0x3B, 0x31, 0x33, 0x28, 0x1C, 0x1B, 0x23, 0x2B,
	0x34, 0x32, 0x3A, 0x61, 0x15, 0x57, 0x5A, 0x29,
	0x14, 0x1A, 0x22, 0x21, 0x2A, 0x32, 0x31, 0x3A,
	0x41, 0x49, 0, 0x11, 0x29, 0x11, 0x41, 0x49,
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

	if (ev.type == SDL_KEYUP) {
		if (code == 0x83 || code == 0x61 || code == 0x7E || code == 0x7D || code == 0x7C) {
			queue_scan(0xF0);
			queue_scan(code);
		} else if (sc == SDL_SCANCODE_RETURN2 || sc == SDL_SCANCODE_RCTRL || sc == SDL_SCANCODE_RALT || sc == SDL_SCANCODE_LGUI || sc == SDL_SCANCODE_RGUI || sc == SDL_SCANCODE_APPLICATION) {
			queue_scan_ext(0xF0);
			queue_scan_ext(code);
		} else {
			queue_scan(0xF0);
			queue_scan(code);
		}
	} else {
		queue_scan(code);
	}
}

void PS2Keyboard::tick(avr_t *avr)
{
	if (!m_sending) {
		if (m_tx.empty()) return;

		uint8_t byte = m_tx.front();
		m_tx.pop();

		uint8_t parity = 1;
		for (int i = 0; i < 8; i++) parity ^= (byte >> i) & 1;

		m_frame = (1u << 0) | ((uint16_t)byte << 1) | ((uint16_t)parity << 9) | (1u << 10);
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
		m_data_state = (m_frame >> m_frame_idx) & 1;

		PS2_LOG("  data=" << (int)m_data_state << " bit=" << (int)m_frame_idx);

		avr_raise_irq(
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1),
			m_data_state);
	} else {
		m_clk_state = true;

		PS2_LOG("  clock falling edge");

		avr_raise_irq(
			avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2),
			0);

		m_frame_idx++;

		if (m_frame_idx >= m_frame_bits) {
			m_sending = false;
			m_clk_state = true;
			m_data_state = true;

			PS2_LOG("TX complete, idle");

			avr_raise_irq(
				avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2),
				1);
			avr_raise_irq(
				avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 1),
				1);
		}
	}
}

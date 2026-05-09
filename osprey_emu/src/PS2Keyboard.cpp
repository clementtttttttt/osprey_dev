#include "PS2Keyboard.h"
#include <simavr/sim_avr.h>
#include <simavr/avr_ioport.h>
#include <iostream>
#include <iomanip>

#define PS2_DEBUG
#ifdef PS2_DEBUG
#define PS2_LOG(x) std::cerr << "[PS2] " << x << std::endl
#else
#define PS2_LOG(x)
#endif

static uint8_t sdl_to_ps2_code(SDL_Scancode sc)
{
	switch (sc) {
		case SDL_SCANCODE_A: return 0x1C;
		case SDL_SCANCODE_B: return 0x32;
		case SDL_SCANCODE_C: return 0x21;
		case SDL_SCANCODE_D: return 0x23;
		case SDL_SCANCODE_E: return 0x24;
		case SDL_SCANCODE_F: return 0x2B;
		case SDL_SCANCODE_G: return 0x34;
		case SDL_SCANCODE_H: return 0x33;
		case SDL_SCANCODE_I: return 0x43;
		case SDL_SCANCODE_J: return 0x3B;
		case SDL_SCANCODE_K: return 0x42;
		case SDL_SCANCODE_L: return 0x4B;
		case SDL_SCANCODE_M: return 0x3A;
		case SDL_SCANCODE_N: return 0x31;
		case SDL_SCANCODE_O: return 0x44;
		case SDL_SCANCODE_P: return 0x4D;
		case SDL_SCANCODE_Q: return 0x15;
		case SDL_SCANCODE_R: return 0x2D;
		case SDL_SCANCODE_S: return 0x1B;
		case SDL_SCANCODE_T: return 0x2C;
		case SDL_SCANCODE_U: return 0x3C;
		case SDL_SCANCODE_V: return 0x2A;
		case SDL_SCANCODE_W: return 0x1D;
		case SDL_SCANCODE_X: return 0x22;
		case SDL_SCANCODE_Y: return 0x35;
		case SDL_SCANCODE_Z: return 0x1A;
		case SDL_SCANCODE_1: return 0x16;
		case SDL_SCANCODE_2: return 0x1E;
		case SDL_SCANCODE_3: return 0x26;
		case SDL_SCANCODE_4: return 0x25;
		case SDL_SCANCODE_5: return 0x2E;
		case SDL_SCANCODE_6: return 0x36;
		case SDL_SCANCODE_7: return 0x3D;
		case SDL_SCANCODE_8: return 0x3E;
		case SDL_SCANCODE_9: return 0x46;
		case SDL_SCANCODE_0: return 0x45;
		case SDL_SCANCODE_RETURN: return 0x5A;
		case SDL_SCANCODE_ESCAPE: return 0x76;
		case SDL_SCANCODE_BACKSPACE: return 0x66;
		case SDL_SCANCODE_TAB: return 0x0D;
		case SDL_SCANCODE_SPACE: return 0x29;
		case SDL_SCANCODE_MINUS: return 0x4E;
		case SDL_SCANCODE_EQUALS: return 0x55;
		case SDL_SCANCODE_LEFTBRACKET: return 0x54;
		case SDL_SCANCODE_RIGHTBRACKET: return 0x5B;
		case SDL_SCANCODE_BACKSLASH: return 0x5D;
		case SDL_SCANCODE_NONUSHASH: return 0x5D;
		case SDL_SCANCODE_SEMICOLON: return 0x4C;
		case SDL_SCANCODE_APOSTROPHE: return 0x52;
		case SDL_SCANCODE_GRAVE: return 0x0E;
		case SDL_SCANCODE_COMMA: return 0x41;
		case SDL_SCANCODE_PERIOD: return 0x49;
		case SDL_SCANCODE_SLASH: return 0x4A;
		case SDL_SCANCODE_CAPSLOCK: return 0x58;
		case SDL_SCANCODE_F1: return 0x05;
		case SDL_SCANCODE_F2: return 0x06;
		case SDL_SCANCODE_F3: return 0x04;
		case SDL_SCANCODE_F4: return 0x0C;
		case SDL_SCANCODE_F5: return 0x03;
		case SDL_SCANCODE_F6: return 0x0B;
		case SDL_SCANCODE_F7: return 0x83;
		case SDL_SCANCODE_F8: return 0x0A;
		case SDL_SCANCODE_F9: return 0x01;
		case SDL_SCANCODE_F10: return 0x09;
		case SDL_SCANCODE_F11: return 0x78;
		case SDL_SCANCODE_F12: return 0x07;
		case SDL_SCANCODE_SCROLLLOCK: return 0x7E;
		case SDL_SCANCODE_INSERT: return 0x70;
		case SDL_SCANCODE_HOME: return 0x6C;
		case SDL_SCANCODE_PAGEUP: return 0x7D;
		case SDL_SCANCODE_DELETE: return 0x71;
		case SDL_SCANCODE_END: return 0x69;
		case SDL_SCANCODE_PAGEDOWN: return 0x7A;
		case SDL_SCANCODE_RIGHT: return 0x74;
		case SDL_SCANCODE_LEFT: return 0x6B;
		case SDL_SCANCODE_DOWN: return 0x72;
		case SDL_SCANCODE_UP: return 0x75;
		case SDL_SCANCODE_NUMLOCKCLEAR: return 0x77;
		case SDL_SCANCODE_KP_DIVIDE: return 0x4A;
		case SDL_SCANCODE_KP_MULTIPLY: return 0x7C;
		case SDL_SCANCODE_KP_MINUS: return 0x7B;
		case SDL_SCANCODE_KP_PLUS: return 0x79;
		case SDL_SCANCODE_KP_ENTER: return 0x5A;
		case SDL_SCANCODE_KP_1: return 0x69;
		case SDL_SCANCODE_KP_2: return 0x72;
		case SDL_SCANCODE_KP_3: return 0x7A;
		case SDL_SCANCODE_KP_4: return 0x6B;
		case SDL_SCANCODE_KP_5: return 0x73;
		case SDL_SCANCODE_KP_6: return 0x74;
		case SDL_SCANCODE_KP_7: return 0x6C;
		case SDL_SCANCODE_KP_8: return 0x75;
		case SDL_SCANCODE_KP_9: return 0x7D;
		case SDL_SCANCODE_KP_0: return 0x70;
		case SDL_SCANCODE_KP_PERIOD: return 0x71;
		case SDL_SCANCODE_NONUSBACKSLASH: return 0x61;
		case SDL_SCANCODE_APPLICATION: return 0x2F;
		case SDL_SCANCODE_LCTRL: return 0x14;
		case SDL_SCANCODE_LSHIFT: return 0x12;
		case SDL_SCANCODE_LALT: return 0x11;
		case SDL_SCANCODE_LGUI: return 0x1F;
		case SDL_SCANCODE_RCTRL: return 0x14;
		case SDL_SCANCODE_RSHIFT: return 0x59;
		case SDL_SCANCODE_RALT: return 0x11;
		case SDL_SCANCODE_RGUI: return 0x27;
		default: return 0;
	}
}

PS2Keyboard::PS2Keyboard()
	: m_frame(0)
	, m_frame_bits(0)
	, m_frame_idx(0)
	, m_clk_counter(0)
	, m_clk_state(true)
	, m_data_state(true)
	, m_sending(false)
	, m_idle(false)
	, m_clock_pending(false)
	, m_inter_frame_delay(0)
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

	uint8_t code = sdl_to_ps2_code(sc);
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
		if (!m_idle) {
			m_idle = true;
			ps2_set_pin(avr, 'D', 2, true);
			ps2_set_pin(avr, 'B', 1, true);
		}
		if (m_inter_frame_delay) {
			m_inter_frame_delay--;
			return;
		}
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
		m_clk_state = false;
		m_data_state = true;
		m_clock_pending = false;

		PS2_LOG("TX byte 0x" << std::hex << (int)byte << std::dec << " parity=" << (int)parity);
	}

	m_clk_counter--;

	if (m_clk_counter > 0) return;

	if (m_clk_state) {
		m_clk_state = false;

		ps2_set_pin(avr, 'D', 2, true);
		m_frame_idx++;
		PS2_LOG("  clock rising edge, sample bit=" << (int)(m_frame_idx - 1) << "  PINB=" << std::hex << (int)avr->data[0x36] << " PIND=" << (int)avr->data[0x30] << std::dec);

		if (m_frame_idx >= m_frame_bits) {
			m_sending = false;
			m_clk_state = true;
			m_data_state = true;
			m_inter_frame_delay = 2000;

			ps2_set_pin(avr, 'D', 2, true);
			ps2_set_pin(avr, 'B', 1, true);
			PS2_LOG("TX complete, idle  PINB=" << std::hex << (int)avr->data[0x36] << " PIND=" << (int)avr->data[0x30] << std::dec);

			return;
		}
		m_clk_counter = PS2_CLK_HALF_PERIOD;
	} else {
		if (m_clock_pending) {
			m_clock_pending = false;
			m_clk_state = true;

			ps2_set_pin(avr, 'D', 2, false);
			PS2_LOG("  clock falling edge  PINB=" << std::hex << (int)avr->data[0x36] << " PIND=" << (int)avr->data[0x30] << std::dec);

			m_clk_counter = PS2_CLK_HALF_PERIOD - 1;
		} else {
			m_clock_pending = true;
			m_data_state = (m_frame >> m_frame_idx) & 1;

			ps2_set_pin(avr, 'B', 1, m_data_state);
			PS2_LOG("  data=" << (int)m_data_state << " bit=" << (int)m_frame_idx << "  PINB=" << std::hex << (int)avr->data[0x36] << " PIND=" << (int)avr->data[0x30] << std::dec);

			m_clk_counter = 1;
		}
	}
}

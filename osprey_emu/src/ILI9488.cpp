#include "ILI9488.h"
#include <iostream>
#include <iomanip>
#include <cstring>

static uint8_t ilitab_cmd_params(uint8_t cmd) {
	switch (cmd) {
		case 0x2A: return 4;
		case 0x2B: return 4;
		case 0x30: return 4;
		case 0x33: return 6;
		case 0x36: return 1;
		case 0x37: return 2;
		case 0x3A: return 1;
		case 0x44: return 2;
		case 0x51: return 1;
		case 0x53: return 1;
		case 0x55: return 1;
		case 0x5E: return 1;
		case 0xB0: return 4;
		case 0xB1: return 4;
		case 0xB2: return 4;
		case 0xB3: return 4;
		case 0xB4: return 1;
		case 0xB5: return 3;
		case 0xB6: return 3;
		case 0xB7: return 1;
		case 0xBE: return 1;
		case 0xC0: return 2;
		case 0xC1: return 1;
		case 0xC2: return 1;
		case 0xC3: return 1;
		case 0xC4: return 1;
		case 0xC5: return 4;
		case 0xC6: return 1;
		case 0xC8: return 10;
		case 0xC9: return 1;
		case 0xCA: return 1;
		case 0xCB: return 1;
		case 0xCC: return 1;
		case 0xCD: return 1;
		case 0xCE: return 1;
		case 0xCF: return 1;
		case 0xD0: return 3;
		case 0xD1: return 2;
		case 0xD7: return 1;
		case 0xE0: return 15;
		case 0xE1: return 15;
		case 0xE2: return 2;
		case 0xE3: return 2;
		case 0xE9: return 2;
		case 0xF2: return 1;
		case 0xF7: return 4;
		case 0xF8: return 2;
		case 0xF9: return 2;
		case 0xFB: return 1;
		case 0xFC: return 2;
		case 0xFF: return 3;
		default: return 0;
	}
}

ILI9488::ILI9488(uint32_t fb[480][320])
	: m_fb(fb)
	, m_state(ST_IDLE)
	, m_cmd(0)
	, m_param_idx(0)
	, m_param_count(0)
	, m_col_start(0), m_col_end(0)
	, m_page_start(0), m_page_end(0)
	, m_col(0), m_page(0)
	, m_pixel_format(0x06)
	, m_madctl(0)
	, m_tfa(0), m_vsa(480), m_bfa(0)
	, m_vscroll_start(0)
	, m_pixel_idx(0)
	, m_dc(false)
	, m_cs(true)
	, m_debug(false)
{
}

const char *ILI9488::cmd_name(uint8_t cmd)
{
	switch (cmd) {
	case 0x00: return "NOP";
	case 0x01: return "SWRESET";
	case 0x04: return "RDDID";
	case 0x09: return "RDDSDR";
	case 0x0A: return "RDDPM";
	case 0x0B: return "RDDMADCTL";
	case 0x0C: return "RDDCOLMOD";
	case 0x0D: return "RDDIM";
	case 0x0E: return "RDDSM";
	case 0x0F: return "RDDSDR";
	case 0x10: return "SLPIN";
	case 0x11: return "SLPOUT";
	case 0x12: return "PTLON";
	case 0x13: return "NORON";
	case 0x20: return "INVOFF";
	case 0x21: return "INVON";
	case 0x22: return "ALLPOFF";
	case 0x23: return "ALLPON";
	case 0x28: return "DISPOFF";
	case 0x29: return "DISPON";
	case 0x2A: return "CASET";
	case 0x2B: return "PASET";
	case 0x2C: return "RAMWR";
	case 0x2E: return "RAMRD";
	case 0x30: return "PTLAR";
	case 0x33: return "VSCRDEF";
	case 0x34: return "TEOFF";
	case 0x35: return "TEON";
	case 0x36: return "MADCTL";
	case 0x37: return "VSCRSADD";
	case 0x38: return "IDMOFF";
	case 0x39: return "IDMON";
	case 0x3A: return "COLMOD";
	case 0x3C: return "RAMWRC";
	case 0x3E: return "RAMRDC";
	case 0x44: return "WTSCAN";
	case 0x51: return "WRDISBV";
	case 0x53: return "WRCTRLD";
	case 0x55: return "WRCABC";
	case 0xB0: return "IFMODE";
	case 0xB1: return "FRMCTR1";
	case 0xB2: return "FRMCTR2";
	case 0xB3: return "FRMCTR3";
	case 0xB4: return "INVCTR";
	case 0xB5: return "DISSET";
	case 0xB6: return "DISCTR";
	case 0xB7: return "ETMOD";
	case 0xBE: return "HSLANO";
	case 0xC0: return "PWCTR1";
	case 0xC1: return "PWCTR2";
	case 0xC2: return "PWCTR3";
	case 0xC3: return "PWCTR4";
	case 0xC4: return "PWCTR5";
	case 0xC5: return "VMCTR";
	case 0xC6: return "CABCCTR1";
	case 0xE0: return "PGAMCTRL";
	case 0xE1: return "NGAMCTRL";
	case 0xF2: return "ADJCTL2";
	case 0xF7: return "ADJCTL3";
	case 0xF8: return "ADJCTL4";
	case 0xF9: return "ADJCTL5";
	case 0xFC: return "ADJCTL6";
	case 0xFF: return "ADJCTL7";
	default: return "???";
	}
}

void ILI9488::set_dc(bool high)
{
	if (m_debug) std::cerr << "[ILI9488] DC=" << high << std::endl;
	m_dc = high;
}

void ILI9488::set_cs(bool high)
{
	if (m_debug) std::cerr << "[ILI9488] CS=" << high << std::endl;
	m_cs = high;
	if (high) {
		m_pixel_idx = 0;
		m_param_idx = 0;
	}
}

void ILI9488::set_debug(bool on)
{
	m_debug = on;
}

void ILI9488::write(uint8_t data)
{
	if (m_cs) return;
	if (!m_dc) {
		if (m_state == ST_PIXEL_WRITE && m_pixel_idx > 0) {
			if (m_debug) std::cerr << "[ILI9488] pixel write ended (" << m_pixel_idx << " trailing bytes)" << std::endl;
			m_state = ST_IDLE;
			m_pixel_idx = 0;
		}
		m_cmd = data;
		m_param_idx = 0;
		m_param_count = ilitab_cmd_params(data);
		if (m_debug) std::cerr << "[ILI9488] CMD 0x" << std::hex << (int)data << " (" << cmd_name(data) << ") params=" << (int)m_param_count << std::dec << std::endl;
		if (m_param_count == 0) {
			exec_cmd();
		} else {
			m_state = ST_CMD_PARAMS;
		}
		return;
	}
	else{

		if (m_state == ST_PIXEL_WRITE) {
			m_pixel_buf[m_pixel_idx++] = data;
//														std::cerr << "PIX WRITE! "<< std::hex << (uint16_t)m_pixel_format << std::endl;

			if (m_pixel_format == 0x61) {

				write_rgb111(m_pixel_buf[0]);
				m_pixel_idx = 0;

			} else {

				if (m_pixel_idx >= 3) {
					write_rgb666(m_pixel_buf[0], m_pixel_buf[1], m_pixel_buf[2]);
					m_pixel_idx = 0;
				}
			}
			return;
		}

	if (m_state == ST_CMD_PARAMS) {
		m_params[m_param_idx++] = data;
			if (m_debug) std::cerr << "[ILI9488]   param[" << (int)(m_param_idx - 1) << "] = 0x" << std::hex << (int)data << std::dec << std::endl;
			if (m_param_idx >= m_param_count) {
				exec_cmd();
				m_state = ST_IDLE;
				m_param_idx = 0;
			}
			return;
		}
	}
	
}

void ILI9488::exec_cmd()
{
	switch (m_cmd) {
		case 0x01:
			if (m_debug) std::cerr << "[ILI9488]   -> SWRESET" << std::endl;
			reset();
			break;
		case 0x2A:
			m_col_start = (m_params[0] << 8) | m_params[1];
			m_col_end   = (m_params[2] << 8) | m_params[3];
			m_col = m_col_start;
			if (m_debug) std::cerr << "[ILI9488]   -> CASET " << m_col_start << ".." << m_col_end << std::endl;
			break;
		case 0x2B:
			m_page_start = (m_params[0] << 8) | m_params[1];
			m_page_end   = (m_params[2] << 8) | m_params[3];
			m_page = m_page_start;
			if (m_debug) std::cerr << "[ILI9488]   -> PASET " << m_page_start << ".." << m_page_end << std::endl;
			break;
		case 0x2C:
			m_col = m_col_start;
			m_page = m_page_start;
			m_state = ST_PIXEL_WRITE;
			m_pixel_idx = 0;
			if (m_debug) std::cerr << "[ILI9488]   -> RAMWR start (" << std::hex<<(int)m_pixel_format << ") @(" << std::dec<<m_col << "," << m_page << ")" << std::endl;
			break;
		case 0x3A:
			m_pixel_format = m_params[0];
			if (m_debug) std::cerr << "[ILI9488]   -> COLMOD 0x" << std::hex << (int)m_pixel_format << std::dec << std::endl;
			break;
		case 0x3C:
			m_state = ST_PIXEL_WRITE;
			m_pixel_idx = 0;
			if (m_debug) std::cerr << "[ILI9488]   -> RAMWRC continue @(" << m_col << "," << m_page << ")" << std::endl;
			break;
	case 0x33:
		m_tfa = (m_params[0] << 8) | m_params[1];
		m_vsa = (m_params[2] << 8) | m_params[3];
		m_bfa = (m_params[4] << 8) | m_params[5];
		if (m_debug) std::cerr << "[ILI9488]   -> VSCRDEF TFA=" << m_tfa << " VSA=" << m_vsa << " BFA=" << m_bfa << std::endl;
		break;
	case 0x36:
		m_madctl = m_params[0];
		if (m_debug) std::cerr << "[ILI9488]   -> MADCTL 0x" << std::hex << (int)m_madctl << std::dec << std::endl;
		break;
	case 0x37:
		m_vscroll_start = (m_params[0] << 8) | m_params[1];
		if (m_debug) std::cerr << "[ILI9488]   -> VSCRSADD " << m_vscroll_start << std::endl;
		break;
	default:
		break;
	}
}

SDL_RendererFlip ILI9488::get_flip() const
{
	SDL_RendererFlip f = SDL_FLIP_NONE;
	if (!!(m_madctl & 0x40) ^ !!(m_madctl & 0x08))
		f = (SDL_RendererFlip)(f | SDL_FLIP_HORIZONTAL);
	if (!!(m_madctl & 0x80) ^ !!(m_madctl & 0x20) ^ !!(m_madctl & 0x04))
		f = (SDL_RendererFlip)(f | SDL_FLIP_VERTICAL);
	return f;
}

double ILI9488::get_angle() const
{
	return (m_madctl & 0x10) ? 90.0 : 0.0;
}

void ILI9488::write_rgb111(uint8_t rgb)
{
	if (m_page < 480 && m_col < 320) {
		uint8_t r8 = (rgb & 0x20) ? 0xFF : 0x00;
		uint8_t g8 = (rgb & 0x10) ? 0xFF : 0x00;
		uint8_t b8 = (rgb & 0x08) ? 0xFF : 0x00;
		m_vram[m_page][m_col] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
	}

	m_col++;
	if (m_col > m_col_end) {
		m_col = m_col_start;
		m_page++;
		if (m_page > m_page_end)
			m_page = m_page_start;
	}

	if (m_page < 480 && m_col < 320) {
		uint8_t r8 = (rgb & 0x04) ? 0xFF : 0x00;
		uint8_t g8 = (rgb & 0x02) ? 0xFF : 0x00;
		uint8_t b8 = (rgb & 0x01) ? 0xFF : 0x00;
		m_vram[m_page][m_col] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
	}

	m_col++;
	if (m_col > m_col_end) {
		m_col = m_col_start;
		m_page++;
		if (m_page > m_page_end)
			m_page = m_page_start;
	}
}

void ILI9488::write_rgb666(uint8_t r, uint8_t g, uint8_t b)
{
	if (m_page < 480 && m_col < 320) {
		uint8_t r8 = r & 0xFC;
		uint8_t g8 = g & 0xFC;
		uint8_t b8 = b & 0xFC;
		m_vram[m_page][m_col] = (r8 << 24) | (g8 << 16) | (b8 << 8) | 0xFF;
	}

	m_col++;
	if (m_col > m_col_end) {
		m_col = m_col_start;
		m_page++;
		if (m_page > m_page_end) {
			m_page = m_page_start;
		}
	}
}

void ILI9488::reset()
{
	m_state = ST_IDLE;
	m_cmd = 0;
	m_param_idx = 0;
	m_param_count = 0;
	m_col_start = 0;
	m_col_end = 0;
	m_page_start = 0;
	m_page_end = 0;
	m_col = 0;
	m_page = 0;
	m_pixel_format = 0x06;
	m_madctl = 0;
	m_tfa = 0;
	m_vsa = 480;
	m_bfa = 0;
	m_vscroll_start = 0;
	m_pixel_idx = 0;
	for (int i = 0; i < 480; i++)
		for (int j = 0; j < 320; j++)
			m_vram[i][j] = 0;
}

void ILI9488::flush()
{
	for (uint16_t gram_y = 0; gram_y < 480; gram_y++) {
		uint16_t display_y = gram_y;
		if (m_vsa > 0 && gram_y >= m_tfa && gram_y < m_tfa + m_vsa)
			display_y = m_tfa + ((gram_y - m_vscroll_start + m_vsa) % m_vsa);

		std::memcpy(m_fb[display_y], m_vram[gram_y], 320 * sizeof(uint32_t));
	}
}

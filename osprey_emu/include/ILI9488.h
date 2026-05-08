#ifndef ILI9488_H
#define ILI9488_H

#include <cstdint>

class ILI9488 {
	uint32_t (*m_fb)[320];

	enum state_t {
		ST_IDLE,
		ST_CMD_PARAMS,
		ST_PIXEL_WRITE
	};
	state_t m_state;

	uint8_t m_cmd;
	uint8_t m_params[4];
	uint8_t m_param_idx;
	uint8_t m_param_count;

	uint16_t m_col_start, m_col_end;
	uint16_t m_page_start, m_page_end;
	uint16_t m_col, m_page;

	uint8_t m_pixel_format;
	uint8_t m_madctl;

	uint8_t m_pixel_buf[3];
	uint8_t m_pixel_idx;

	bool m_dc;
	bool m_cs;

	void exec_cmd();
	void write_rgb111(uint8_t rgb);
	void write_rgb666(uint8_t r, uint8_t g, uint8_t b);

	const char *cmd_name(uint8_t cmd);

public:
	ILI9488(uint32_t fb[480][320]);

	void set_dc(bool high);
	void set_cs(bool high);
	void write(uint8_t data);
	void reset();
	void set_debug(bool on);

private:
	bool m_debug;
};

#endif

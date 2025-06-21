/*
 * test.c
 *
 * Copyright (c) 2012 - 2017 Thomas Buck <xythobuz@xythobuz.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h> 
#include "ili9488.h"
#include "osprey_font.h"
#include "ps2.h"
#include "serial.h"

/** \example test.c
 *  Initializes all available UART Modules.
 *  Then prints a welcome message on each and waits
 *  for incoming characters, which will be repeated
 *  on the UART module they were received on.
 */
 /*
 uint8_t font[][] = 
 {	0,0,0,0,0,0,
 */
 typedef enum screen_3bit_colors_t{
	 S3_BLACK=0, S3_BLUE, S3_GREEN, S3_CYAN,
	 S3_RED, S3_PURPLE, S3_YELLOW, S3_WHITE
	 
 } screen_3bit_colors_t;


typedef uint32_t screen_24bit_colors_t;

#define VIA1_CA1 (1<<4)
#define VIA1_CA2 (1<<5)

#define VIA0_CA1 (1<<7)
#define VIA0_CA2 (1<<3)

#define LCD_DC (1<<3)
#define LCD_CS (1<<4)
#define LCD_CLK (1<<7)
#define LCD_RST (1<<2)
void ack_6522(void) {

    //6522 data taken = low;
    	_delay_us(30);

    PORTD &= ~VIA1_CA1;
    _delay_us(30);
    PORTD |= VIA1_CA1;
	_delay_us(30);
}

static inline void wait_for_6522(void) {
    while (PIND & (VIA1_CA2 )) {

    } 	

}



static inline void screen_write(char cData)
{
/* Start transmission */
SPDR = cData;
/* Wait for transmission complete */
while(!(SPSR & _BV(SPIF)));
}

void writedata(uint8_t c){
	PORTB |= LCD_DC;
	PORTB &= ~(LCD_CS);
	
	screen_write(c);
	
	PORTB |= LCD_CS;
}

void writecommand(uint8_t c){
	PORTB &= ~LCD_DC;
	
	PORTB &= ~LCD_CS;
	screen_write(c);
	PORTB |= LCD_CS;
}


void writecmddata_p(uint8_t c, const uint8_t *restrict d, size_t cnt){
	PORTB &= ~LCD_DC;
	PORTB &= ~LCD_CS;
	screen_write(c);
	
	
	PORTB |= LCD_DC;
	
	for(size_t i=0;i<cnt;++i){
		screen_write(pgm_read_byte(&d[i]));
	}
	

	
	PORTB |= LCD_CS;
}

void writecmddata_s(uint8_t c, const uint8_t d){
	PORTB &= ~LCD_DC;
	PORTB &= ~LCD_CS;
	screen_write(c);
	
	
	PORTB |= LCD_DC;
	
		screen_write(d);
	
	

	
	PORTB |= LCD_CS;
}

void writecmddata(uint8_t c, const uint8_t *restrict d, size_t cnt){
	PORTB &= ~LCD_DC;
	PORTB &= ~LCD_CS;
	screen_write(c);
	
	
	PORTB |= LCD_DC;
	
	for(size_t i=0;i<cnt;++i){
		screen_write(d[i]);
	}
	

	
	PORTB |= LCD_CS;
}

static inline void screen_set_3bit(void){
		writecmddata_s(0x3A, 0x61);      // Interface Pixel Format	  //3 bit
}

static inline void screen_set_18bit(void){
		writecmddata_s(0x3A, 0x66);      // Interface Pixel Format	  //18 bit

}


uint8_t test_spcr;

			const uint8_t init_data[] PROGMEM= {
		0,3,9,8,0x16,0xa,0x3f,0x78,0x4c,0x09,0xa,0x8,0x16,0x1a,0xf};
			const uint8_t init_data_2  [] PROGMEM = {
		0,0x16,0x19,3,0xf,5,0x32,0x45,0x46,0x4,0xe,0xd,0x35,0x37,0x0f
	};
void screen_init(void)
{
	/* Set MOSI SCK and /SS output*/
	DDRB = (1<< DDB5)|(1<< DDB7) | LCD_DC | LCD_CS | LCD_CLK | LCD_RST;
	PORTB = LCD_CS ;
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<< SPE)|(1<< MSTR) ;
	SPSR |= 1<<SPI2X; //DOUBLE THE SPEED

	PORTB |= LCD_RST;
	_delay_ms(5);
		PORTB &= ~(LCD_RST);
	_delay_ms(20);
	PORTB |= LCD_RST;
	_delay_ms(150);

		


	writecmddata_p(0xe0, init_data, sizeof(init_data));



	writecmddata_p(0xe1, init_data_2, sizeof(init_data_2));



	writecommand(0XC0);      //Power Control 1
	writedata(0x17);    //Vreg1out
	writedata(0x15);    //Verg2out

	writecmddata_s(0xC1,0x41);      //Power Control 2    //VGH,VGL

	writecmddata(0xC5, (const uint8_t[]){0,0x12,0x80},3);      //Power Control 3  //Vcom


	writecmddata_s(0x36,0x48);      //Memory Access

	writecmddata_s(0x3A,0x66);      // Interface Pixel Format 	  //18 bit

	writecmddata_s(0XB0,0);      // Interface Mode Control			 //SDO NOT USE

	writecmddata_s(0xB1,0xa0);      //Frame rate    //60Hz

	writecmddata_s(0xB4,2);      //Display Inversion Control    //2-dot

	writecommand(0XB6);      //Display Function Control  RGB/MCU Interface Control

	writedata(0x02);    //MCU
	writedata(0x02);    //Source,Gate scan dieection

	writecmddata_s(0XE9,0);      // Set Image Functio// Disable 24 bit data

	writecmddata(0xf7, (const uint8_t[]){0xa9,0x51,0x2c,0x82}, 4); // Adjust Control // D7 stream, loose



#define ILI9488_SLPOUT  0x11
#define ILI9488_DISPON  0x29
  writecommand(ILI9488_SLPOUT);    //Exit Sleep
  
  _delay_ms(120);
    writecommand(ILI9488_DISPON);    //Display on
}


char screen_read(void)				/* SPI read data function */
{
	SPDR = 0xFF;
	while(!(SPSR & _BV(SPIF)));	/* Wait till reception complete */
	return(SPDR);			/* Return received data */
}

uint8_t screen_read_cmd(uint8_t c){
	uint8_t index = 0;
	PORTB &= ~(LCD_DC | LCD_CS);
	screen_write(0xd9); //magik
	PORTB |= LCD_DC;
	screen_write(0x10 + index);
	PORTB |= LCD_CS;
	
	PORTB &= ~(LCD_DC | LCD_CS | LCD_CLK);
	
	screen_write(c);
	
	PORTB |= LCD_DC;
	uint8_t r = screen_read();
	PORTB |= LCD_CS;
	return r;
	
}

void wait_for_write_6522(void){
	while(PIND & (VIA0_CA2)){
	}
}

void write_ack_6522(void){
		PORTD &= ~VIA0_CA1;

		wait_for_write_6522();
				PORTD |= VIA0_CA1;

}

void screen_setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1,
 uint16_t y1) {
	 
	 
	 uint8_t caset_data[] = {
			x0 >> 8, x0 & 0xff, x1 >> 8, x1 & 0xff
	 };
	 
	  writecmddata(ILI9488_CASET, caset_data, 4); // Column addr set

	
	uint8_t paset_data[] = {
		y0 >> 8, y0 & 0xff, y1 >> 8, y1 & 0xff
	};
	
	

  writecmddata(ILI9488_PASET, paset_data, 4); // Row addr set


	 
 }
 
void write3BitColor(uint8_t color, uint8_t color2){
	
	color <<= 3;
	color &= 0b111000;
	color2 &= 0b111;
	color |= color2;
	screen_write(color);
}

screen_24bit_colors_t screen_convert_16bit_color_to_24(uint16_t color){
	screen_24bit_colors_t ret = 0;
	uint8_t *restrict raw = (uint8_t *restrict)&ret;
	
	raw[2] = (color & 0xF800) >> 11;
	raw[1] = (color & 0x07E0) >> 5;
	raw[0] = color & 0x001F;

 
	raw[2] = (raw[2] * 255) / 31;
	raw[1] = (raw[1] * 255) / 63;
	raw[0] = (raw[0] * 255) / 31;
	
	return ret;
}

static inline screen_24bit_colors_t screen_convert_3bit_color_to_24(screen_3bit_colors_t in){
	screen_24bit_colors_t ret = 0;
	uint8_t *restrict raw = (uint8_t *restrict)&ret;
	
	raw[2] = (in&0b100)?0xff:0;
	raw[1] = (in&0b10)?0xff:0;
	raw[0] = (in&0b1)?0xff:0;


	
	return ret;
}
 
static inline void write24BitColor(screen_24bit_colors_t in){



  uint8_t *restrict raw = (uint8_t *restrict)&in;

  screen_write(raw[2]);
  screen_write(raw[1]);
  screen_write(raw[0]);
  // #endif
}

void draw_char(uint32_t x, uint32_t y, screen_24bit_colors_t color,screen_24bit_colors_t bg, char c){
	uint8_t rows[8];
	
	memcpy_P(rows, &osprey_font_bits[((size_t)c)*8], 8);
		screen_setAddrWindow(x,y,x+7, y+7); //each char is 8 wide 8 tall, have to be subtracted by 1 for this 
	
  writecommand(ILI9488_RAMWR); // write to RAM

	PORTB |= LCD_DC;
	PORTB &= ~(LCD_CS);
	
	
	
	for(y=0; y<8; ++y) {
		uint8_t mask0 = 1;
		while(mask0) {
			// spiwrite(hi);
			// spiwrite(lo);
			// spiwrite(0); // added for 24 bit
			screen_24bit_colors_t color1 = (rows[y] & mask0)? color:bg;
			mask0 <<= 1;
			write24BitColor(color1);
		}
	}


	PORTB |= LCD_CS;
}

void fastrect(int16_t x, int16_t y, int16_t w, int16_t h, screen_3bit_colors_t color){
	if((x>=320) && (y>=480)) return;
	
	if((x+w-1) >= 320) w = 320-x;
	if((y+h-1) >= 480) h = 480 - y;

	screen_set_3bit();
	screen_setAddrWindow(x,y,x+w-1, y+h-1);
	
	writecommand(ILI9488_RAMWR);
	
	PORTB |= LCD_DC;
	PORTB &= ~(LCD_CS);
		  for(y=0; y<h; ++y) {

    for(x=0; x<(w/2); ++x) {
      // spiwrite(hi);
      // spiwrite(lo);
      // spiwrite(0); // added for 24 bit
      write3BitColor(color,color);
    }
  }

	PORTB |= LCD_CS;
		screen_set_18bit();

}

void draw_char_fast(uint32_t x, uint32_t y, screen_3bit_colors_t color,screen_3bit_colors_t bg, char c){
	uint8_t rows[8];
		screen_set_3bit();

	memcpy_P(rows, &osprey_font_bits[((size_t)c)*8], 8);
		screen_setAddrWindow(x,y,x+7, y+7); //each char is 8 wide 8 tall, have to be subtracted by 1 for this 
	
  writecommand(ILI9488_RAMWR); // write to RAM

	PORTB |= LCD_DC;
	PORTB &= ~(LCD_CS);

	
	for(y=0; y<8; ++y) {
		uint8_t mask0 = 1;
		while(mask0) {
			// spiwrite(hi);
			// spiwrite(lo);
			// spiwrite(0); // added for 24 bit
			uint8_t color1 = (rows[y] & mask0)? color:bg;
			mask0 <<= 1;
			uint8_t color2 = (rows[y] & mask0)? color:bg;
			mask0 <<= 1;  
			write3BitColor(color1,color2);
		}
	}


	PORTB |= LCD_CS;

			screen_set_18bit();

}
static uint16_t tx=0, ty=0, tscroll_off=0;

void draw_scroll_screen(uint32_t vsp){
	uint8_t def_dat[] = {0,0,480>>8, 480&0xff,0,0};
	writecmddata(ILI9488_VSCRDEF, def_dat, sizeof(def_dat));
	
	uint8_t scroll_dat[] = {vsp >> 8, vsp & 0xff};
	writecmddata(ILI9488_VSCRSADD, scroll_dat, sizeof(scroll_dat));
	

	
	
}

screen_3bit_colors_t term_fg = S3_BLACK;
screen_3bit_colors_t term_bg = S3_GREEN;

void draw_string(const char *restrict c){
	screen_24bit_colors_t fg = screen_convert_3bit_color_to_24(term_fg);
	screen_24bit_colors_t bg = screen_convert_3bit_color_to_24(term_bg);
	while(*c){
		if(ty >= (480/8)){
			ty = 480/8-1;
			fastrect(0, tscroll_off, 320, 8, S3_GREEN);
			draw_scroll_screen(tscroll_off = ((tscroll_off + 8)% 480));

		}
		if(*c == 0xa){
			tx = 0;
			++ty;
			++c;
			continue;
		}
		if(*c == 0xd){
			tx=0;
			++c;
			continue;
		}
		if(*c == 127){
			--tx;
			++c;
			continue;
		}
		//draw_char_fast(tx*8, (ty*8 + tscroll_off)%480, term_fg, term_bg, *c);
		draw_char(tx*8, (ty*8 + tscroll_off)%480, fg, bg, *c);
		++tx;
		if(tx >= (320/8)){
			++ty;
			tx = 0;
		}
		
		++c;
	}
}
 



void rect(int16_t x, int16_t y, int16_t w, int16_t h, screen_24bit_colors_t color){
	if((x>=320) && (y>=480)) return;
	

	if((x+w-1) >= 320) w = 320-x;
	if((y+h-1) >= 480) h = 480 - y;
	
	screen_setAddrWindow(x,y,x+w-1, y+h-1);
	
	PORTB |= LCD_DC;
	PORTB &= ~(LCD_CS);

	  for(y=0; y<h; y++) {
		  	    for(x=0; x<w; ++x) {

      // spiwrite(hi);
      // spiwrite(lo);
      // spiwrite(0); // added for 24 bit
      write24BitColor(color);
    }
  }

	PORTB |= LCD_CS;
}

ISR (INT0_vect) { /* PS2 interrupt */


			ps2_read();
	
	


}

typedef enum cmd_t {
    CMD_PUTC = 1,
    CMD_DUMMY = 2,
    CMD_GETC,
    CMD_FPUSH,
    CMD_SETC,
    CMD_SETFGBG
    
} cmd_t;

uint8_t get_6522(void) {
    wait_for_6522();

    uint8_t ret = PINC;
        ack_6522();

    
    return ret;
}

float float_stack[16];
uint8_t float_sp = 0;;
void float_push(float in){
	float_stack[float_sp++] = in;
	
}


uint16_t get_short_6522(void){
	uint16_t ret = 0;
	uint8_t *raw = (uint8_t*)&ret;
	raw[1] = get_6522();
	raw[0] = get_6522();
	
	return ret;
}

float get_float_6522(void){
	float ret = 0;
	uint8_t *raw = (uint8_t*)&ret;
	
	for(int i=4;i>0;--i){
		raw[i-1] = get_6522();		//big endian conversion
	}
	
	return ret;
}

uint8_t sprites[16][8][4] __attribute__((section(".data"))); //2 pixels per byte

int main(void) {
    
    // Initialize UART modules
    for (int i = 0; i < serialAvailable(); i++) {
        serialInit(i, 12);
    }

    // Enable Interrupts
    sei();
	screen_init();
	ps2_init();
	
	
	screen_write(0xff);
	screen_write(0xff);
	screen_write(0xff);
	screen_write(0xff);

	fastrect(0,0,320,480,S3_GREEN);

    // Print Welcome Message
    serialWriteString(0, "\r\nOsprey SIO serial interface initialised\n\r");
	draw_string("Osprey SIO graphics interface initialised\n");

    //GICR |= 1 << INT0; //enable int0 so we can receive data ready from 6522
    //MCUCR &= (1 << ISC00) | (1 << ISC01); //int0 triggers at low level
	

    PORTD |= VIA1_CA1; //6522 data taken = high;
    DDRD |= VIA1_CA1;
    
    PORTD |= VIA0_CA1; //6522_0 data ready = high;
    DDRD |= VIA0_CA1;
    
    DDRA = 0xff; //6522 receive port high


    // Wait for incoming bytes from 6522
    while (1) {
        cmd_t in = get_6522();
        switch (in) {
			case CMD_PUTC:
				char b = get_6522();
				serialWrite(0, b);
				char test[] = {b, 0};
				draw_string(test);
			break;
			case CMD_FPUSH:
				float_push(get_float_6522());
				
			case CMD_DUMMY:
				break;
				
			case CMD_SETC: //two args: y and x
				ty = get_short_6522();
				tx = get_short_6522();
				break;
			case CMD_SETFGBG:
				uint8_t in_fgbg = get_6522(); //5:3 = fg,2:0 = bg
				term_fg = (in_fgbg >> 3) & 0b111;
				term_bg = in_fgbg& 0b111;
				break;
			case CMD_GETC:
				unsigned char c = 0;
			    while(!serialHasChar(0) && !(c=ps2_buf_pull())){
				
				}
				if(serialHasChar(0)){
					PORTA = serialGet(0); 
				}
				else{
					PORTA = c;
				}
				write_ack_6522();
				break;
			default:;
				{
					char test[7];
					snprintf(test, 7, "?%.2x\r\n", in);
					serialWriteString(0, test);
				}
				break;
		}



    }
    return 0;
}

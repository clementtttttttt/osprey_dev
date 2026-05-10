#include <exception>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <SDL.h>
#include <simavr/sim_elf.h>
#include <simavr/avr_ioport.h>
#include <simavr/avr_spi.h>

#include <simavr/avr_uart.h>
#include "CPU6809.h"
#include "VIA6522.h"
#include "ILI9488.h"
#include "PS2Keyboard.h"

#include <fstream>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>

#define NO_AVR_TRACE

enum {
	IRQ_UART_PTY_BYTE_IN = 0,
	IRQ_UART_PTY_BYTE_OUT,
	IRQ_UART_PTY_COUNT
};



avr_t * sio = NULL;
int ticks = 0;
bool debug_step = false;
bool debug_advance = false;

class InitError : public std::exception
{
    std::string msg;
public:
    InitError();
    InitError( const std::string & );
    virtual ~InitError() throw();
    virtual const char * what() const throw();
};

InitError::InitError() :
    exception(),
    msg( SDL_GetError() )
{
}

InitError::InitError( const std::string & m ) :
    exception(),
    msg( m )
{
}

InitError::~InitError() throw()
{
}


const static int LOW_MEM_SZ = 32768;
const static int ROM_SZ = 32768;

uint8_t low_mem[LOW_MEM_SZ];
uint8_t rom[ROM_SZ];
VIA6522 VIA0;
VIA6522 VIA1;

const char * InitError::what() const throw()
{
    return msg.c_str();
}

class SDL
{
    SDL_Window * m_window;
    SDL_Renderer * m_renderer;
    uint32_t pixels[480][320];

public:
    SDL( Uint32 flags = 0 );
    virtual ~SDL();
    void draw();
    virtual bool poll_events(PS2Keyboard* kbd = nullptr);
    uint32_t (*get_fb())[320] { return pixels; }

    SDL_Texture *fb;
};

SDL::SDL( Uint32 flags )
{
    if ( SDL_Init( flags ) != 0 )
        throw InitError();

    if ( SDL_CreateWindowAndRenderer( 320*2, 480*2, SDL_WINDOW_SHOWN,
                                      &m_window, &m_renderer ) != 0 )
        throw InitError();
    SDL_SetHint (SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetHint (SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitor");
    SDL_SetWindowTitle(m_window, "OSPREY PORTBALE COMPUTING UNIT EMULATOR");
    fb = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 320, 480);
}

SDL::~SDL()
{
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_Quit();
}

bool SDL::poll_events(PS2Keyboard* kbd){
    SDL_Event ev;
    while(SDL_PollEvent(&ev)){
        if(ev.type == SDL_QUIT){
            return false;
        }
        if(ev.type == SDL_KEYDOWN && !ev.key.repeat){
            if(ev.key.keysym.scancode == SDL_SCANCODE_F1){
                debug_step = !debug_step;
                std::cout << "[DEBUG] " << (debug_step ? "STEP MODE (F2=step, F1=run)" : "RUNNING") << std::endl;
                continue;
            }
            if(ev.key.keysym.scancode == SDL_SCANCODE_F2){
                debug_advance = true;
                continue;
            }
        }
        if(kbd) kbd->handle_event(ev);
    }
    return true;
}
void SDL::draw()
{
    SDL_UpdateTexture(fb, NULL, pixels, 320*sizeof(uint32_t));
    SDL_RenderClear( m_renderer );
    SDL_RenderCopy(m_renderer, fb, NULL, NULL);
    // Show the window
    SDL_RenderPresent( m_renderer );


}

uint8_t rmf(uint16_t addr);

static void avr_debug_wait_advance() {
	while(debug_step && !debug_advance){
		SDL_PumpEvents();
		SDL_Event ev;
		while(SDL_PollEvent(&ev)){
			if(ev.type == SDL_QUIT) exit(0);
			if(ev.type == SDL_KEYDOWN && !ev.key.repeat){
				if(ev.key.keysym.scancode == SDL_SCANCODE_F1){
					debug_step = false;
					std::cout << "[DEBUG] RUNNING" << std::endl;
				}
				if(ev.key.keysym.scancode == SDL_SCANCODE_F2){
					debug_advance = true;
				}
				if(ev.key.keysym.scancode == SDL_SCANCODE_F3){
					int old_flags = fcntl(STDIN_FILENO, F_GETFL);
					fcntl(STDIN_FILENO, F_SETFL, old_flags & ~O_NONBLOCK);
					std::cout << "[6809 MEM] addr> " << std::flush;
					uint32_t addr;
					std::cin >> std::hex >> addr;
					std::cin.clear();
					std::cout << "[6809 MEM] 0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr
					          << " = 0x" << std::setw(2) << (int)rmf((uint16_t)addr) << std::dec << std::endl;
					fcntl(STDIN_FILENO, F_SETFL, old_flags);
				}
			}
		}
		SDL_Delay(10);
	}
	debug_advance = false;
}

#ifndef NO_AVR_TRACE
	static void avr_debug_tick(int i) {
	uint8_t sreg = sio->data[0x5f];
	uint8_t gicr = sio->data[0x5b];
	uint8_t mcucr = sio->data[0x55];
	uint8_t ddra = sio->data[0x3a], pina = sio->data[0x39];
	uint8_t ddrb = sio->data[0x37], pinb = sio->data[0x36];
	uint8_t ddrc = sio->data[0x34], pinc = sio->data[0x33];
	uint8_t ddrd = sio->data[0x31], pind = sio->data[0x30];
				    std::cout.clear();

	std::cout << "[AVR] tick=" << std::dec << ticks << ":" << i
	          << " pc=0x" << std::hex << sio->pc
	          << " sreg=0x" << (int)sreg << " ["
	          << ((sreg & 0x80) ? 'I' : '-')
	          << ((sreg & 0x40) ? 'T' : '-')
	          << ((sreg & 0x20) ? 'H' : '-')
	          << ((sreg & 0x10) ? 'S' : '-')
	          << ((sreg & 0x08) ? 'V' : '-')
	          << ((sreg & 0x04) ? 'N' : '-')
	          << ((sreg & 0x02) ? 'Z' : '-')
	          << ((sreg & 0x01) ? 'C' : '-')
	          << "]"
	          << " gicr=0x" << (int)gicr << " INT1=" << ((gicr & 0x80) ? 'Y' : 'N')
	          << " mcucr=0x" << (int)mcucr << " ISC1=" << ((mcucr >> 2) & 3)
	          << " ddra=0x" << (int)ddra << "/pin=0x" << (int)pina
	          << " ddrb=0x" << (int)ddrb << "/pin=0x" << (int)pinb
	          << " ddrc=0x" << (int)ddrc << "/pin=0x" << (int)pinc
	          << " ddrd=0x" << (int)ddrd << "/pin=0x" << (int)pind
	          << std::dec << std::endl;
	if(std::cout.bad())  std::cerr << "[DEBUG] cout badbit set!" << std::endl;
	if(std::cout.fail()) std::cerr << "[DEBUG] cout failbit set!" << std::endl;

}
#endif

void via_din_hook(struct avr_irq_t * irq, uint32_t value, void * pa){
	static uint64_t last_cycle = 0;
	if(sio->cycle == last_cycle) return;
	last_cycle = sio->cycle;
	uint8_t porta_val = sio->data[AVR_IO_TO_DATA(0x1B)];
			std::cout << "PORTA: "<<porta_val << std::endl;

	VIA0.ext_write_porta(0xff, porta_val);
}
void via1_ca1_hook(struct avr_irq_t * irq, uint32_t value, void * pa){
	
		VIA1.ext_set_ca1(value);
}

void via0_ca1_hook(struct avr_irq_t * irq, uint32_t value, void * pa){
		VIA0.ext_set_ca1(value);

}

static const char * irq_names[IRQ_UART_PTY_COUNT] = {
	[IRQ_UART_PTY_BYTE_IN] = "8<uart_pty.in",
	[IRQ_UART_PTY_BYTE_OUT] = "8>uart_pty.out",
};

static void ser_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param){

    std::cout << (char)value << std::flush;
}

static ILI9488 *lcd = NULL;
static PS2Keyboard ps2kbd;

static void spi_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param){

	if (lcd) lcd->write((uint8_t)value);
}

static void dc_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param){

	if (lcd) lcd->set_dc(value != 0);

}

static void cs_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param){

	if (lcd) lcd->set_cs(value != 0);

}


uint8_t rmf(uint16_t addr){

    const static uint16_t PAGESEL_START = 0xc000;

    if(addr < LOW_MEM_SZ){
        return low_mem[addr];
    }
    else
    if(addr >=0x8000 && addr < 0xc000){
		if(!(addr & 0x10)){ //a4 not set == via0
			return VIA0.reg_read((size_t)(addr&0xf));
		}
		if((addr & 0x10)){ //a4 set == via1
			return VIA1.reg_read((size_t)(addr&0xf));
		}
	}

    else if(addr >= PAGESEL_START){
        uint32_t mapped_addr = addr - PAGESEL_START;

        //TODO: pagesel selection
        return rom[mapped_addr + 0x4000];
    }
    
    
    
    return 0xff;
}

void wmf(uint16_t addr, uint8_t data){
	if(addr < LOW_MEM_SZ){
        low_mem[addr] = data;
    }
	else
	if(addr >=0x8000 && addr < 0xc000){
		if(!(addr & 0x10)){ //a4 not set == via0
			VIA0.reg_write((size_t)(addr&0xf), data);
		}
		else if((addr & 0x10)){ //a4 set == via1
			VIA1.reg_write((size_t)(addr&0xf), data);
		}
	}

}

void via0_ca2_cb(bool in){

	avr_irq_t *pin = avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('D'), 3);
	avr_raise_irq(pin, in);

	if(in) sio->data[AVR_IO_TO_DATA(0x10)] |=  (1 << 3);
	else   sio->data[AVR_IO_TO_DATA(0x10)] &= ~(1 << 3);

	
		if(in == false){
		//	debug_step = true;

		}
}

void via1_ca2_cb(bool in){

	avr_irq_t *pin = avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('D'), 5);
	avr_raise_irq(pin, in);

}

void via0_pbw_cb(uint8_t in){
	for (int i = 0; i < 8; i++) {
		avr_irq_t *pin = avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('C'), i);
		avr_raise_irq(pin, (in >> i) & 1);
	}
	
	//std::cout << "PB WRITE " << (uint16_t)in << std::endl;
}
CPU6809 sys_cpu(rmf, wmf);


void irq_cb(){
	sys_cpu.assert_irq();
}



avr_irq_t *ser_irq;
avr_irq_t *spi_irq;

int main( int argc, char * argv[] )
{

    SDL sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER );

    fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

    lcd = new ILI9488(sdl.get_fb());
	//lcd->set_debug(true);

    elf_firmware_t firm = {{0}};
    elf_read_firmware("test.elf", &firm);

    sio = avr_make_mcu_by_name("atmega16");
    const static int AVR_FREQ = 16000000;
    sio->frequency = AVR_FREQ; //16mhz

    avr_init(sio);
    avr_load_firmware(sio, &firm);

    //connect all pins on port a to 6522 (atmega write, 6522 in)


	for (int i = 0; i < 8; i++){
		avr_irq_register_notify(
			avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('A'), i),
			via_din_hook,
			NULL);
	}
			
		avr_irq_register_notify(
			avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('D'), 4),
			via1_ca1_hook,
			NULL);
			
		avr_irq_register_notify(
			avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('D'), 7),
			via0_ca1_hook,
			NULL);
    //connect serial
    ser_irq = avr_alloc_irq(&sio->irq_pool, 0, IRQ_UART_PTY_COUNT, irq_names);
	avr_irq_register_notify(ser_irq + IRQ_UART_PTY_BYTE_IN, ser_hook, (void*)1234);


	const char *spi_names = "ili9488 spi irq";
    //connect spi
    spi_irq = avr_alloc_irq(&sio->irq_pool, 0, 1, &spi_names);
	avr_irq_register_notify(spi_irq, spi_hook, (void*)1234);


	//connect via1 ca2 
	VIA1.set_ca2_w_cb(via1_ca2_cb);

	//connect via0 ca2 
	VIA0.set_ca2_w_cb(via0_ca2_cb);
	

	//conect via0 callbacks
	VIA0.set_pb_w_cb(via0_pbw_cb);
	

    uint32_t f=0; //flag for avr uart
    //disable studio dump
	avr_ioctl(sio, AVR_IOCTL_UART_GET_FLAGS('0'), &f);
	f &= ~AVR_UART_FLAG_STDIO;
	avr_ioctl(sio, AVR_IOCTL_UART_SET_FLAGS('0'), &f);

	{
		avr_irq_t * src = avr_io_getirq(sio, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUTPUT);
		avr_connect_irq(src, ser_irq);
	}

    sio->data[AVR_IO_TO_DATA(0x10)] = 0x00; //portd pull-ups disabled

	avr_irq_t * spisrc = avr_io_getirq(sio, AVR_IOCTL_SPI_GETIRQ(0), SPI_IRQ_OUTPUT);
	avr_connect_irq(spisrc, spi_irq);

	avr_irq_register_notify(
		avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('B'), 3),
		dc_hook,
		NULL);

	avr_irq_register_notify(
		avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('B'), 4),
		cs_hook,
		NULL);

    std::ifstream rom_file("rom", std::ios::binary);
    rom_file.read(reinterpret_cast<char*>(&rom[0]), ROM_SZ);


	
	//connect cpu irq
	
	VIA1.set_on_irq_cb(irq_cb);
	VIA0.set_on_irq_cb(irq_cb);



    uint64_t perf_freq = SDL_GetPerformanceFrequency();
    uint64_t last_counter = SDL_GetPerformanceCounter();

    while(sdl.poll_events(&ps2kbd)){
        ++ticks;


        {
            char c;
            while(read(STDIN_FILENO, &c, 1) > 0){
				if(c == '\n') c = 0xd;
				if(c != 0){
					avr_irq_t *pin = avr_io_getirq(sio, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_INPUT);
					avr_raise_irq(pin, c);
				}
            }
        }

        uint64_t now = SDL_GetPerformanceCounter();
        uint64_t elapsed_ticks = now - last_counter;
        last_counter = now;

        uint32_t avr_cycles = (uint32_t)(elapsed_ticks * AVR_FREQ / perf_freq);

        const uint32_t max_cycles_per_frame = AVR_FREQ / 30;
        if (avr_cycles > max_cycles_per_frame) avr_cycles = max_cycles_per_frame;

        for(uint32_t i=0;i<avr_cycles;++i){
            avr_run(sio);
            if(debug_step){
#ifndef NO_AVR_TRACE
				                        avr_debug_tick(i);
#endif
				 	avr_debug_wait_advance();

            }
            if(i % 8 == 0){ //2 mhz for 6809

				sys_cpu.run_cycles(1);
				VIA0.phi2_tick();
				VIA1.phi2_tick();
				ps2kbd.tick(sio);

			}
        }

        lcd->flush();
        sdl.draw();

    }

    return 0;
}

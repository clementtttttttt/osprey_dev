#include <exception>
#include <string>
#include <iostream>
#include <SDL.h>
#include <CPUAtmega16.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/avr_ioport.h>
#include <simavr/avr_uart.h>
#include "CPU6809.h"
#include <fstream>

enum {
	IRQ_UART_PTY_BYTE_IN = 0,
	IRQ_UART_PTY_BYTE_OUT,
	IRQ_UART_PTY_COUNT
};



avr_t * sio = NULL;
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
    virtual bool poll_events();

    SDL_Texture *fb;
};

SDL::SDL( Uint32 flags )
{
    if ( SDL_Init( flags ) != 0 )
        throw InitError();

    if ( SDL_CreateWindowAndRenderer( 320, 480, SDL_WINDOW_SHOWN,
                                      &m_window, &m_renderer ) != 0 )
        throw InitError();
    SDL_SetHint (SDL_HINT_RENDER_VSYNC, "1");
    SDL_SetWindowTitle(m_window, "OSPREY PORTBALE COMPUTING UNIT EMULATOR");
    fb = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 320, 480);
}

SDL::~SDL()
{
    SDL_DestroyWindow( m_window );
    SDL_DestroyRenderer( m_renderer );
    SDL_Quit();
}

bool SDL::poll_events(){

    SDL_Event ev;

    SDL_PollEvent(&ev);

    if(ev.type == SDL_QUIT){
        return false;
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

int ticks = 0;

void via_din_hook(struct avr_irq_t * irq, uint32_t value, void * pa){

}

static const char * irq_names[IRQ_UART_PTY_COUNT] = {
	[IRQ_UART_PTY_BYTE_IN] = "8<uart_pty.in",
	[IRQ_UART_PTY_BYTE_OUT] = "8>uart_pty.out",
};

static void ser_hook(
		struct avr_irq_t * irq,
		uint32_t value,
		void * param){

    std::cout << (char)value;
}

const static int LOW_MEM_SZ = 32768;
const static int ROM_SZ = 32768;

uint8_t low_mem[LOW_MEM_SZ];
uint8_t rom[ROM_SZ];

uint8_t rmf(uint16_t addr){

    const static uint16_t PAGESEL_START = 0xc000;

    if(addr < LOW_MEM_SZ){
        return low_mem[addr];
    }
    else if(addr >= PAGESEL_START){
        uint32_t mapped_addr = addr - PAGESEL_START;

        //TODO: pagesel selection
        return rom[mapped_addr + 0x4000];
    }
    else return 0xff;
}

void wmf(uint16_t addr, uint8_t data){


}



avr_irq_t *ser_irq;
int main( int argc, char * argv[] )
{
    SDL sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER );

    elf_firmware_t firm = {{0}};
    elf_read_firmware("test.elf", &firm);

    sio = avr_make_mcu_by_name("atmega16");
    const static int AVR_FREQ = 16000000;
    sio->frequency = AVR_FREQ; //16mhz

    avr_init(sio);
    avr_load_firmware(sio, &firm);

    //connect all pins on port a to 6522 (atmega write, 6522 in)


	for (int i = 0; i < 8; i++)
		avr_irq_register_notify(
			avr_io_getirq(sio, AVR_IOCTL_IOPORT_GETIRQ('A'), i),
			via_din_hook,
			NULL);

    //connect serial
    ser_irq = avr_alloc_irq(&sio->irq_pool, 0, IRQ_UART_PTY_COUNT, irq_names);
	avr_irq_register_notify(ser_irq + IRQ_UART_PTY_BYTE_IN, ser_hook, (void*)1234);

    uint32_t f=0; //flag for avr uart
    //disable studio dump
	avr_ioctl(sio, AVR_IOCTL_UART_GET_FLAGS('0'), &f);
	f &= ~AVR_UART_FLAG_STDIO;
	avr_ioctl(sio, AVR_IOCTL_UART_SET_FLAGS('0'), &f);

	avr_irq_t * src = avr_io_getirq(sio, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUTPUT);
    avr_connect_irq(src, ser_irq);

    sio->data[AVR_IO_TO_DATA(0x10)] = 0xff; //set portd to fully high



    std::ifstream rom_file("rom", std::ios::binary);
    rom_file.read(reinterpret_cast<char*>(&rom[0]), ROM_SZ);


    CPU6809 sys_cpu(rmf, wmf);

    while(sdl.poll_events()){
        ++ticks;


        for(int i=0;i<AVR_FREQ/60;++i){
            avr_run(sio);
            if(i % 8 == 0) //2 mhz for 6809
            sys_cpu.run_cycles(1);
        }

        sdl.draw();

    }

    return 0;
}

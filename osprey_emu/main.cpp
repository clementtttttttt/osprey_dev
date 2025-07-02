#include <exception>
#include <string>
#include <iostream>
#include <SDL.h>
#include <CPUAtmega16.h>

extern "C"{
    #include "CPUAtmega16.h"
}
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
    CPUAtmega16 sio;

int main( int argc, char * argv[] )
{

        SDL sdl( SDL_INIT_VIDEO | SDL_INIT_TIMER );
    sio.loadHex("../sio/sio.hex");

    while(sdl.poll_events()){
        ++ticks;
        sio.cycle(999);
        sdl.draw();
    }

    return 0;
}

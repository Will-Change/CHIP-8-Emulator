#include <iostream>
#include <chrono>
#include <thread>
#include "stdint.h"
#include "SDL2/SDL.h"



#include "Chip.hpp"

using namespace std;

//this does NOT check out but stil is the most boring part
// Keypad keymap
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char **argv) {

    // Command usage
    if (argc != 2) {
        cout << "Usage: chip8 <ROM file>" << endl;
    }

    Chip8 chip8 = Chip8();// initialise Chip8, again i should have used a contructor, but who even cares lol
    int w = 1024;// Window width
    int h = 512;// Window height, these should have been global, but the program isnt really based entirely on sdl so its fine. will keep in mind next time around :)

    //GOD I HATE SDL SO MUCH, when will i get the hang of this
    // The window we'll be rendering to
    SDL_Window* window = NULL;

    // Initialize SDL
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        exit(1);
    }
    // Create window
    window = SDL_CreateWindow(
            "CHIP-8 Emulator",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            w, h, SDL_WINDOW_SHOWN
    );
    if (window == NULL){
        printf( "Window could not be created! SDL_Error: %s\n",
                SDL_GetError() );
        exit(2);
    }

    //our renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, w, h);

    //this texture stores our buffer
            SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);

    // temp pixel buffer, the word buffer is losing all its value rn
    uint32_t pixels[2048];


    load:
    // this is easiest way i found, is a little slower than the others but in the grand scheme of things doesnt matter
    if(chip8.loadROM(argc, argv[1]))

    //basic emulation loop
    while (true) {
        chip8.emulate_cycle();

        //process SDL events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);

            // Process keydown events
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                    exit(0);//YAH I KNOWWWWWWWWW DONT WORRY ABOUT IT

                if (e.key.keysym.sym == SDLK_F1)
                    goto load;      // *gasp*, a goto statement!
                                    // Used to reset/reload ROM
                //got this goto statement from this guy on github, will include that in the read me file

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.key[i] = 0;
                    }
                }
            }
        }

        // If draw occurred, redraw SDL screen
        if (chip8.drawFlag) {
            chip8.drawFlag = false;
            //i talked about drawFlag in depth before so dont worry about it much
            // Store pixels in temporary buffer
            for (int i = 0; i < 2048; ++i) {
                uint8_t pixel = chip8.gfx[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }
            // update SDL texture
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            // clean it upppp
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // sleep to slow down emulation speed
        std::this_thread::sleep_for(std::chrono::microseconds(1200));

    }
}

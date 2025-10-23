#include <cmath>
#include <iostream>
#include <core/core.hpp>
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"

namespace NV1Sim
{

Game game = {0};               

bool Game_Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
        return false; 

    game.settings.screen_x = 1024;
    game.settings.screen_y = 768;

    if (!SDL_CreateWindowAndRenderer(APP_SIGNON_STRING, game.settings.screen_x, game.settings.screen_y, 0, &game.window, &game.renderer))
        return false;

   
    game.render_target = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, game.settings.screen_x, game.settings.screen_y);

    if (!game.render_target)
    {
        std::cout << "Failed to create render target " << SDL_GetError() << std::endl;
        Game_Shutdown();
    }
    
    game.running = true; 
    game.tickrate = 60;

    return true; 
}

void Game_PumpEvents()
{
    SDL_Event next_event; 

    if (SDL_PollEvent(&next_event))
    {
        bool w_down = false, a_down = false, s_down = false, d_down = false;
        bool left_down = false, right_down = false, up_down = false, down_down = false;  

        switch (next_event.type)
        {
           
            case SDL_EVENT_KEY_UP:
                key_state[next_event.key.scancode] = false; 
                break; 
            case SDL_EVENT_KEY_DOWN:
                key_state[next_event.key.scancode] = true;
                break; 
            case SDL_EVENT_QUIT:
                game.running = false; 
                break;

        }
    }
}

void Game_Tick()
{
   
}

bool Game_Shutdown()
{
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);

    SDL_Quit();

    exit(0);

    return true;
}
}
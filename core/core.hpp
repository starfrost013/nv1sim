/* Core SDL init/shutdown code */
#pragma once
#include "SDL3/SDL_scancode.h"
#include <nv1sim.hpp>

namespace NV1Sim
{
    /* Game states */
    enum GameState 
    {
        Running = 0,
    };

    /* Game settings */
    struct GameSettings
    {  
        uint32_t screen_x;
        uint32_t screen_y;
    };

    /* Game information that changes on level change */
    struct GameInfo 
    {
        uint32_t level_id; 
    };

    #define NS_PER_SECOND 1000000000

    struct Game
    {
        SDL_Window* window;         // SDL Window

        SDL_Renderer* renderer;     // SDL Renderer

        SDL_Texture* render_target;  // Texture that gets blited to the display

        uint32_t tickrate;          // The game's tickrate

        uint64_t last_tick_time;    // last frame time in nanoseconds (returned by SDL_GetTicksNS)

        GameInfo info;              // Level-specific game information

        GameSettings settings;      // Game settings

        bool running;               // Determines if the game is running.
    };

    extern Game game; 

    // Core functionality
    bool Game_Init();
    void Game_PumpEvents();
    void Game_Tick();               // Run each tick
    bool Game_Shutdown();

    // Rendering
    void Game_RenderLevel();

    // Input

    // scancodes are a terrible idea
    extern bool key_state[];

    bool Input_KeyIsDown(uint32_t scancode);
}
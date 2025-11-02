/* "Wolfenstein 3D" styled Digital Differential Analyzer raycaster (Test Project for SDL3) */

#include "SDL3/SDL_timer.h"
#include "core/core.hpp"
#include <NV1Sim.hpp>
#include <core/core.hpp>
#include <core/ui/ui.hpp>
#include <iostream>

namespace NV1Sim
{
    int32_t nv1sim_main(int32_t argc, char** argv)
    {
        Logging_Init();
        Game_Init();

        while (game.running)
        {
            uint64_t time_now = SDL_GetTicksNS();
        
            Game_PumpEvents();
            Game_StartRenderUI();

            if (time_now > (game.last_tick_time + (NS_PER_SECOND)))
            {
                Game_Tick();
                game.last_tick_time = time_now;
            }

            SDL_RenderClear(game.renderer);

            Game_RenderLevel();
            Game_RenderUI();

            Game_EndRenderUI();
            SDL_RenderPresent(game.renderer);
        }

        Game_Shutdown();

        return 0; 
    }
}

int32_t main(int32_t argc, char** argv)
{
    return NV1Sim::nv1sim_main(argc, argv);
}
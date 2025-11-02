#include <cmath>
#include <iostream>
#include <core/core.hpp>
#include <nv/nv1.hpp>
#include "SDL3/SDL_error.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "core/ui/ui.hpp"
#include "imgui_impl_sdl3.h"

namespace NV1Sim
{
    NV1* gpu;

    Game game = {0};               

    bool Game_Init()
    {
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
            return false; 

        game.settings.screen_x = 1024;
        game.settings.screen_y = 768;
    
        game.window = SDL_CreateWindow(APP_SIGNON_STRING, game.settings.screen_x, game.settings.screen_y, 0);

        if (!game.window)
        {
            Logging_LogChannel("Failed to allocate SDL window: %s", LogChannel::Fatal, SDL_GetError());
            return false;
        }
    
        game.gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL, true, nullptr);

        if (!game.gpu_device)
        {
            Logging_LogChannel("Failed to allocate SDL_GPU device: %s", LogChannel::Fatal, SDL_GetError());
            return false; 
        }

        if (!SDL_ClaimWindowForGPUDevice(game.gpu_device, game.window))
        {
            Logging_LogChannel("Failed to claim window for GPU device: %s", LogChannel::Fatal, SDL_GetError());
            return false;
        }
        
        // these must be the same as the InitSDLGPU3 call
        SDL_SetGPUSwapchainParameters(game.gpu_device, game.window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_VSYNC);

        /*
        game.render_target = SDL_CreateTexture(game.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, game.settings.screen_x, game.settings.screen_y);

        if (!game.render_target)
        {
            Logging_LogChannel("Failed to create render target: %s", LogChannel::Fatal, SDL_GetError());
            Game_Shutdown();
        }
        */
        game.running = true; 
        game.tickrate = 60;

        Game_InitUI();

        // we're done, move onto nv1 init
        Logging_LogChannel("Initialising graphics hardware...", LogChannel::Message);

        // initialise the nv1 settings
        GPUSettings settings = {0};

        settings.vram_amount = 0x400000;    // the full 4MB 
        settings.straps = 0x7;              // test 

        gpu = new NV1(settings);

        gpu->Start();

        return true; 
    }

    void Game_PumpEvents()
    {

        SDL_Event next_event; 

        if (SDL_PollEvent(&next_event))
        {
            // first send the event down into UI
            ImGui_ImplSDL3_ProcessEvent(&next_event);
            
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
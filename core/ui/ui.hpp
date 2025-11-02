#pragma once

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

//
// Generic UI core headers
//


namespace NV1Sim
{
    struct AppUI
    {
        const char* name;

        // Immediate-mode gui, so we only need to set up the Ui that was enabled each frame
        void (*SetupFunction)();

        bool enabled;

    };

    extern AppUI AppUIs[];

    void Game_InitUI();
    void Game_StartRenderUI();
    void Game_RenderUI();
    void Game_EndRenderUI();
}
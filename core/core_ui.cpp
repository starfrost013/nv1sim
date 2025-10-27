
//
// core_ui.cpp: Implements the IMGUI stuff
//

#include <core/core.hpp>

#include "imgui.h"
#include "imgui_impl_sdl3.h"

namespace NV1Sim
{
    // Initialise IMGUI v1.92, SDL3+OpenGL3
    void Game_InitUI()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        auto ui = ImGui::GetIO();

        ui.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        // It doesn't need to be fast!
        ImGui_ImplSDL3_InitForSDLGPU(game.window);
    }

    void Game_StartRenderUI()
    {
        // Pump IMGUI events
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void Game_EndRenderUI()
    {
        ImGui::Render();
    }
}



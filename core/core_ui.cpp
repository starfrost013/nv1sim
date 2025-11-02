
//
// core_ui.cpp: Implements the IMGUI stuff
//

#include <core/core.hpp>
#include <core/ui/ui.hpp>

#include "SDL3/SDL_gpu.h"
#include "ui/ui.hpp"


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

        // set up our graphics settings
        ImGui_ImplSDLGPU3_InitInfo info = 
        {
            .Device = game.gpu_device,
            .ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(game.gpu_device, game.window),
            .MSAASamples = SDL_GPU_SAMPLECOUNT_1,
            .SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
            .PresentMode = SDL_GPU_PRESENTMODE_VSYNC,
        };

        ImGui_ImplSDLGPU3_Init(&info);

        // create all UIs

    }

    void Game_StartRenderUI()
    {
        // Pump IMGUI events
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
    }

    void Game_RenderUI()
    {
         uint32_t ui_index = 0; 

        // array is probably a bit faster than STL
        AppUI appui = AppUIs[ui_index];

        while (appui.name != nullptr)
        {
            if (appui.enabled)
                appui.SetupFunction();

            ui_index++; 
            appui = AppUIs[ui_index];
        }
    }

    // This actually performs the UI renders
    void Game_EndRenderUI()
    {
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();

        // Set up our GPU resources (we don't need these anywhere else) around if we need it
        SDL_GPUCommandBuffer* buffer = SDL_AcquireGPUCommandBuffer(game.gpu_device);
        SDL_GPUTexture* swapchain;

        SDL_WaitAndAcquireGPUSwapchainTexture(buffer, game.window, &swapchain, nullptr, nullptr);
       
        // Tell the gpu how to treat our texture
        SDL_GPUColorTargetInfo target_info = {
            .texture = swapchain,
            .mip_level = 0,
            .layer_or_depth_plane = 0,
            .clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_LOAD,
            .store_op = SDL_GPU_STOREOP_STORE,
            .cycle = false,
            
        };

        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, buffer);
        // Set up a render pass

        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(buffer, &target_info, 1, nullptr);
        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, buffer, render_pass);
        SDL_EndGPURenderPass(render_pass);

        SDL_SubmitGPUCommandBuffer(buffer);
    }
}



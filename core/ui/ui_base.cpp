
#include "imgui.h"
#include "imgui_internal.h"
#include <core/core.hpp>
#include <core/ui/ui.hpp>

namespace NV1Sim
{

    void UI_MainCreate()
    {
        ImGui::Begin("Nvidia NV1 Multimedia Accelerator Simulator");
        ImGui::SeparatorText("GPU Meta");
        ImGui::Checkbox("Running", &gpu->state.running);
        //ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        ImGui::Text("Straps = 0x%x", gpu->straps);
        ImGui::Text("PFB_BOOT_0 = 0x%x", gpu->pfb.boot);
        ImGui::Text("PMC_BOOT_0 = 0x%x", gpu->pmc.boot);
        ImGui::Text("Video RAM = %d MB", (gpu->settings.vram_amount >> 20));

        ImGui::SeparatorText("GPU Global State:");
        ImGui::Text("PMC_INTR = 0x%0x", gpu->pmc.intr);
        ImGui::Text("PMC_INTR_EN = 0x%0x", gpu->pmc.intr_en);
        ImGui::Text("PMC_ENABLE = 0x%0x", gpu->pmc.enable);
        ImGui::End();
    }

    AppUI AppUIs[] =
    {
        { "UI_Main", UI_MainCreate, true }, //test
        { nullptr, nullptr },
    };
}
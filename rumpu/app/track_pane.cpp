#include "track_pane.hpp"

#include "imgui.h"
#include "imgui-knobs.h"

namespace securepath::drum {

bool track_pane::draw()
{
    if (ImGuiKnobs::Knob("Volume", &gain_, -6.0f, 6.0f, 0.1f, "%.1f", ImGuiKnobVariant_Tick, 30)) {
    
    }
    if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0)) {
       gain_ = 0;
    }
    //ImGui::Text("Pane");    
    return true;
}

}
#include "track_pane.hpp"

#include "imgui.h"
#include "imgui-knobs.h"

namespace securepath::drum::app {

track_pane::track_pane(std::string name)
: child_window_base(std::move(name), ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

bool track_pane::do_draw()
{
    if (ImGuiKnobs::Knob("Gain", &gain_, -6.0f, 6.0f, 0.1f, "%.1f", ImGuiKnobVariant_Tick, 30)) {
    
    }
    if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0)) {
       gain_ = 0;
    }
    //ImGui::Text("Pane");    
    return true;
}

}
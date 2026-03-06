
#include "section_info_view.hpp"

#include "imgui.h"

namespace securepath::drum::app {

section_info_view::section_info_view()
: child_window_base("section_info_view", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

void section_info_view::set_context(song* s, uint32_t section) {
    song_ = s;
    current_section_ = section;
}

bool section_info_view::do_draw() {
    if (!song_) {
        return false;
    }

    auto* section = song_->find_section(current_section_);
    if (!section) {
        return false;
    }

    ImGui::Text("%s", section->name().c_str());
    ImGui::SameLine();
    ImGui::Text("Length:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    int length = section->length();
    if(ImGui::InputInt("bars", &length, 1)) {
        if(length >= 1)
            section->set_length(length);
    }
    return true;
}

}

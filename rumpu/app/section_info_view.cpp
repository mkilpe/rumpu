
#include "section_info_view.hpp"

#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

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

    // Sync buffer when section changes
    if (current_section_ != 0 && name_buf_ != section->name()) {
        name_buf_ = section->name();
    }
    ImGui::SetNextItemWidth(150);
    if (ImGui::InputText("##section_name", &name_buf_)) {
        section->set_name(name_buf_);
    }
    ImGui::SameLine();
    ImGui::Text("Length:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    int length = section->length();
    if(ImGui::InputInt("bars", &length, 1)) {
        if(length >= 1 && length <= 1024)
            section->set_length(length);
    }
    return true;
}

}

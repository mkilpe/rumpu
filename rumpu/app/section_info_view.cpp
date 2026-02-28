
#include "section_info_view.hpp"

#include "imgui.h"

namespace securepath::drum::app {

void section_info_view::set_context(song const* s, uint32_t section) {
    song_ = s;
    current_section_ = section;
}

void section_info_view::do_draw() {
    if (!song_) {
        return;
    }

    auto const* section = song_->find_section(current_section_);
    if (!section) {
        return;
    }

    ImGui::Text("%s", section->name().c_str());
    ImGui::SameLine();
    ImGui::Text("Length: %u bars", section->length());
}

}

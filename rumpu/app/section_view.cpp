
#include "section_view.hpp"
#include "events.hpp"

#include <securepath/log/log.hpp>
#include "imgui.h"

#include <string>

namespace securepath::drum::app {

section_view::section_view(event_system::event_handler& h)
: handler_(h)
{}

void section_view::set_context(song* s, uint32_t section) {
    song_ = s;
    current_section_ = section;
}

void section_view::do_draw() {
    if (!song_) {
        return;
    }

    auto const& order = song_->section_order();

    if (ImGui::Button("+##add_section")) {
        LOG_TRACE("add_section clicked");
        handler_.emit<event::add_section>();
    }

    for (std::size_t i = 0; i < order.size(); ++i) {
        auto id = order[i];
        bool is_selected = (id == current_section_);

        ImGui::SameLine();

        if (is_selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        std::string label = "Section " + std::to_string(i + 1);
        if (ImGui::Button(label.c_str())) {
            handler_.emit<event::select_section>(id);
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }
    }
}

}

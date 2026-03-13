
#include "add_track_dialog.hpp"
#include "events.hpp"

#include "imgui.h"

namespace securepath::drum::app {

add_track_dialog::add_track_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_track_dialog::open(song* s, std::uint32_t section) {
    song_ = s;
    section_ = section;
    selected_ = -1;
    open_ = true;
}

void add_track_dialog::draw_content() {
    auto const& instruments = song_->instruments();

    float button_height = ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginListBox("##instruments", ImVec2(-FLT_MIN, -button_height))) {
        for (std::size_t i = 0; i < instruments.size(); ++i) {
            bool is_selected = (static_cast<int>(i) == selected_);
            if (ImGui::Selectable(instruments[i].name().c_str(), is_selected)) {
                selected_ = static_cast<int>(i);
            }
        }
        ImGui::EndListBox();
    }

    bool const has_selection = selected_ >= 0 && static_cast<std::size_t>(selected_) < instruments.size();
    if (!has_selection) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Add")) {
        handler_.emit<event::add_track>(section_, static_cast<std::size_t>(selected_));
        ImGui::CloseCurrentPopup();
    }
    if (!has_selection) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
    }
}

bool add_track_dialog::draw() {
    if (open_) {
        ImGui::OpenPopup("Add track");
        open_ = false;
    }

    if (ImGui::BeginPopupModal("Add track", nullptr, ImGuiWindowFlags_None)) {
        draw_content();
        ImGui::EndPopup();
    }
    return true;
}

}

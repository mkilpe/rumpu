
#include "add_track_dialog.hpp"
#include "events.hpp"

#include "imgui.h"

namespace securepath::drum::app {

add_track_dialog::add_track_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_track_dialog::open(song* s, std::uint32_t section) {
    task_ = run(s, section);
}

ui_task add_track_dialog::run(song* s, std::uint32_t section) {
    ImGui::OpenPopup("Add track");
    co_await next_frame{};

    int selected = -1;

    while (true) {
        ImGui::SetNextWindowSize({380, 420}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Add track", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        auto const& instruments = s->instruments();

        float button_height = ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginListBox("##instruments", ImVec2(-FLT_MIN, -button_height))) {
            for (std::size_t i = 0; i < instruments.size(); ++i) {
                bool is_selected = (static_cast<int>(i) == selected);
                if (ImGui::Selectable(instruments[i].name().c_str(), is_selected)) {
                    selected = static_cast<int>(i);
                }
            }
            ImGui::EndListBox();
        }

        bool const has_selection = selected >= 0 && static_cast<std::size_t>(selected) < instruments.size();
        if (!has_selection) {
            ImGui::BeginDisabled();
        }
        bool const do_add = ImGui::Button("Add");
        if (!has_selection) {
            ImGui::EndDisabled();
        }
        if (do_add) {
            handler_.emit<event::add_track>(section, static_cast<std::size_t>(selected));
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }

        ImGui::EndPopup();
        co_await next_frame{};
    }
}

bool add_track_dialog::draw() {
    task_.tick();
    return true;
}

}

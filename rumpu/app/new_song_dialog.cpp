
#include "new_song_dialog.hpp"
#include "events.hpp"

#include <rumpu/core/time_signature.hpp>

#include "imgui.h"

#include <algorithm>
#include <cstring>

namespace securepath::drum::app {

new_song_dialog::new_song_dialog(event_system::event_handler& h)
: handler_(h)
{}

void new_song_dialog::open() {
    open_ = true;
    std::memset(name_, 0, sizeof(name_));
    beats_ = 4;
    beat_type_ = 4;
    tempo_ = 120.0f;
}

bool new_song_dialog::draw() {
    if (open_) {
        ImGui::OpenPopup("New Song");
        open_ = false;
    }

    if (ImGui::BeginPopupModal("New Song", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Song name:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##name", name_, sizeof(name_));

        ImGui::Text("Time signature:");
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Beats##beats", &beats_);
        beats_ = std::clamp(beats_, 1, 32);
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Beat type##type", &beat_type_);
        beat_type_ = std::clamp(beat_type_, 1, 32);

        ImGui::Text("Tempo (BPM):");
        ImGui::SetNextItemWidth(100);
        ImGui::InputFloat("##tempo", &tempo_, 1.0f, 10.0f, "%.1f");
        tempo_ = std::clamp(tempo_, 20.0f, 400.0f);

        if (ImGui::Button("Create")) {
            handler_.emit<event::new_song>(
                std::string(name_),
                time_signature{static_cast<uint16_t>(beats_), static_cast<uint16_t>(beat_type_)},
                tempo_
            );
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return true;
}

}

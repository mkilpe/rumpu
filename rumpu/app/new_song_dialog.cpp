
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
    task_ = run();
}

ui_task new_song_dialog::run() {
    ImGui::OpenPopup("New Song");
    co_await next_frame{};

    char name[256]{};
    int beats = 4;
    int beat_type = 4;
    float tempo = 120.0f;

    while (true) {
        ImGui::SetNextWindowSize({400, 220}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("New Song", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        ImGui::Text("Song name:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##name", name, sizeof(name));

        ImGui::Text("Time signature:");
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Beats##beats", &beats);
        beats = std::clamp(beats, 1, 32);
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("Beat type##type", &beat_type);
        beat_type = std::clamp(beat_type, 1, 32);

        ImGui::Text("Tempo (BPM):");
        ImGui::SetNextItemWidth(100);
        ImGui::InputFloat("##tempo", &tempo, 1.0f, 10.0f, "%.1f");
        tempo = std::clamp(tempo, 20.0f, 400.0f);

        if (ImGui::Button("Create")) {
            handler_.emit<event::new_song>(
                std::string(name),
                time_signature{static_cast<uint16_t>(beats), static_cast<uint16_t>(beat_type)},
                tempo
            );
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

bool new_song_dialog::draw() {
    task_.tick();
    return true;
}

}

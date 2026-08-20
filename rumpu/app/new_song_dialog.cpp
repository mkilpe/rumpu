
#include "new_song_dialog.hpp"
#include "dialog_widgets.hpp"
#include "events.hpp"

#include <rumpu/core/time_signature.hpp>

#include "imgui.h"
#include "imgui_stdlib.h"

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

    std::string name;
    int beats = 4;
    int beat_type = 4;
    float tempo = 120.0f;

    while (true) {
        ImGui::SetNextWindowSize({400, 220}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("New Song", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        song_fields(name, beats, beat_type, tempo);

        if (ImGui::Button("Create")) {
            handler_.emit<event::new_song>(
                name,
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

void new_song_dialog::song_fields(std::string& name, int& beats, int& beat_type, float& tempo) {
    ImGui::Text("Song name:");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##name", &name);

    timing_fields(beats, beat_type, tempo);
}

bool new_song_dialog::draw() {
    task_.tick();
    return true;
}

}

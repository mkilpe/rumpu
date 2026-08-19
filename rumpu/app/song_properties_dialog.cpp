
#include "song_properties_dialog.hpp"
#include "events.hpp"

#include <rumpu/core/time_signature.hpp>

#include "imgui.h"
#include "imgui_stdlib.h"

#include <algorithm>

namespace securepath::drum::app {

song_properties_dialog::song_properties_dialog(event_system::event_handler& h)
: handler_(h)
{}

void song_properties_dialog::open(song const* s) {
    task_ = run(s);
}

ui_task song_properties_dialog::run(song const* s) {
    auto const& info = s->meta_info();

    std::string name = info.name;
    std::string author = info.author;
    std::string notes = info.notes;

    auto ts = s->default_time_signature();
    int beats = ts.beats_in_bar();
    int beat_type = ts.beat_type();
    float tempo = s->default_tempo().value;
    float rand_offset_ms = s->rand_offset().max_ms;
    float rand_volume_percent = s->rand_volume().max_percent;

    ImGui::OpenPopup("Song Properties");
    co_await next_frame{};

    while (true) {
        ImGui::SetNextWindowSize({480, 520}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Song Properties", nullptr, 0)) {
            co_return;
        }

        ImGui::Text("Name:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##name", &name);

        ImGui::Text("Author:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##author", &author);

        ImGui::Text("Notes:");
        float const notes_reserve = ImGui::GetFrameHeightWithSpacing() * 12;
        ImGui::InputTextMultiline("##notes", &notes, ImVec2(-FLT_MIN, -notes_reserve));

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

        ImGui::Separator();
        ImGui::Text("Randomisation:");

        ImGui::Text("Random hit offset (ms):");
        ImGui::SetNextItemWidth(100);
        ImGui::InputFloat("##rand_offset", &rand_offset_ms, 20.0f, 100.0f, "%.1f");
        rand_offset_ms = std::clamp(rand_offset_ms, 0.0f, 500.0f);

        ImGui::Text("Random volume (%%):");
        ImGui::SetNextItemWidth(100);
        ImGui::InputFloat("##rand_volume", &rand_volume_percent, 1.0f, 5.0f, "%.1f");
        rand_volume_percent = std::clamp(rand_volume_percent, 0.0f, 100.0f);

        if (ImGui::Button("OK")) {
            handler_.emit<event::update_song_properties>(
                name,
                author,
                notes,
                time_signature{static_cast<uint16_t>(beats), static_cast<uint16_t>(beat_type)},
                tempo,
                rand_offset_ms,
                rand_volume_percent
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

bool song_properties_dialog::draw() {
    task_.tick();
    return true;
}

}

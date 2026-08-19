
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

namespace {

struct properties_state {
    std::string name;
    std::string author;
    std::string notes;
    int beats{};
    int beat_type{};
    float tempo{};
    float rand_offset_ms{};
    float rand_volume_percent{};

    explicit properties_state(song const& s)
    : name(s.meta_info().name)
    , author(s.meta_info().author)
    , notes(s.meta_info().notes)
    , beats(s.default_time_signature().beats_in_bar())
    , beat_type(s.default_time_signature().beat_type())
    , tempo(s.default_tempo().value)
    , rand_offset_ms(s.rand_offset().max_ms)
    , rand_volume_percent(s.rand_volume().max_percent)
    {
    }
};

void metadata_fields(properties_state& p) {
    ImGui::Text("Name:");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##name", &p.name);

    ImGui::Text("Author:");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##author", &p.author);

    ImGui::Text("Notes:");
    float const notes_reserve = ImGui::GetFrameHeightWithSpacing() * 12;
    ImGui::InputTextMultiline("##notes", &p.notes, ImVec2(-FLT_MIN, -notes_reserve));
}

void timing_fields(properties_state& p) {
    ImGui::Text("Time signature:");
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Beats##beats", &p.beats);
    p.beats = std::clamp(p.beats, 1, 32);
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Beat type##type", &p.beat_type);
    p.beat_type = std::clamp(p.beat_type, 1, 32);

    ImGui::Text("Tempo (BPM):");
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat("##tempo", &p.tempo, 1.0f, 10.0f, "%.1f");
    p.tempo = std::clamp(p.tempo, 20.0f, 400.0f);
}

void randomisation_fields(properties_state& p) {
    ImGui::Separator();
    ImGui::Text("Randomisation:");

    ImGui::Text("Random hit offset (ms):");
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat("##rand_offset", &p.rand_offset_ms, 20.0f, 100.0f, "%.1f");
    p.rand_offset_ms = std::clamp(p.rand_offset_ms, 0.0f, 500.0f);

    ImGui::Text("Random volume (%%):");
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat("##rand_volume", &p.rand_volume_percent, 1.0f, 5.0f, "%.1f");
    p.rand_volume_percent = std::clamp(p.rand_volume_percent, 0.0f, 100.0f);
}

}

ui_task song_properties_dialog::run(song const* s) {
    properties_state state{*s};

    ImGui::OpenPopup("Song Properties");
    co_await next_frame{};

    while (true) {
        ImGui::SetNextWindowSize({480, 520}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Song Properties", nullptr, 0)) {
            co_return;
        }

        metadata_fields(state);
        timing_fields(state);
        randomisation_fields(state);

        if (ImGui::Button("OK")) {
            handler_.emit<event::update_song_properties>(
                state.name,
                state.author,
                state.notes,
                time_signature{static_cast<uint16_t>(state.beats), static_cast<uint16_t>(state.beat_type)},
                state.tempo,
                state.rand_offset_ms,
                state.rand_volume_percent
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

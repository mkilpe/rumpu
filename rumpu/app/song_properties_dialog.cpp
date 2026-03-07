
#include "song_properties_dialog.hpp"
#include "events.hpp"

#include <rumpu/core/time_signature.hpp>

#include "imgui.h"

#include <algorithm>
#include <cstring>

namespace securepath::drum::app {

song_properties_dialog::song_properties_dialog(event_system::event_handler& h)
: handler_(h)
{}

void song_properties_dialog::open(song const* s) {
    open_ = true;
    auto const& info = s->meta_info();
    std::strncpy(name_, info.name.c_str(), sizeof(name_) - 1);
    name_[sizeof(name_) - 1] = '\0';
    std::strncpy(author_, info.author.c_str(), sizeof(author_) - 1);
    author_[sizeof(author_) - 1] = '\0';
    std::strncpy(notes_, info.notes.c_str(), sizeof(notes_) - 1);
    notes_[sizeof(notes_) - 1] = '\0';
    auto ts = s->default_time_signature();
    beats_ = ts.beats_in_bar();
    beat_type_ = ts.beat_type();
    tempo_ = s->default_tempo().value;
}

bool song_properties_dialog::draw() {
    if (open_) {
        ImGui::OpenPopup("Song Properties");
        open_ = false;
    }

    if (ImGui::BeginPopupModal("Song Properties", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##name", name_, sizeof(name_));

        ImGui::Text("Author:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##author", author_, sizeof(author_));

        ImGui::Text("Notes:");
        ImGui::InputTextMultiline("##notes", notes_, sizeof(notes_), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4));

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

        if (ImGui::Button("OK")) {
            handler_.emit<event::update_song_properties>(
                std::string(name_),
                std::string(author_),
                std::string(notes_),
                time_signature{static_cast<uint16_t>(beats_), static_cast<uint16_t>(beat_type_)},
                tempo_
            );
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return true;
}

}

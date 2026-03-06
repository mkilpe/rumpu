#include "track_pane.hpp"
#include "events.hpp"

#include "imgui.h"
#include "imgui-knobs.h"

#include <cmath>

namespace securepath::drum::app {

track_pane::track_pane(std::string name)
: child_window_base(std::move(name), ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

void track_pane::set_context(event_system::event_handler& h, song* s, std::size_t instrument_index) {
    handler_ = &h;
    song_ = s;
    instrument_index_ = instrument_index;
    if(s && instrument_index < s->instruments().size()) {
        float vol = s->instruments()[instrument_index].volume().value;
        gain_ = 20.0f * std::log10(std::max(vol, 1e-6f));
    }
}

bool track_pane::do_draw()
{
    if (song_ && instrument_index_ < song_->instruments().size()) {
        bool muted = song_->instruments()[instrument_index_].volume().mute;

        ImVec4 btn_color = muted
            ? ImVec4(0.7f, 0.2f, 0.2f, 1.0f)
            : ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        ImVec4 btn_hovered = muted
            ? ImVec4(0.8f, 0.3f, 0.3f, 1.0f)
            : ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btn_hovered);

        std::string btn_id = "M##m" + std::to_string(instrument_index_);
        if (ImGui::SmallButton(btn_id.c_str())) {
            drum::volume v = song_->instruments()[instrument_index_].volume();
            v.mute = !muted;
            song_->instruments()[instrument_index_].set_volume(v);
        }

        ImGui::PopStyleColor(2);
        ImGui::SameLine();
    }

    if (ImGuiKnobs::Knob("Gain", &gain_, -6.0f, 6.0f, 0.1f, "%.1f", ImGuiKnobVariant_Tick, 30)) {
        if(song_ && instrument_index_ < song_->instruments().size()) {
            drum::volume v = song_->instruments()[instrument_index_].volume();
            v.value = std::pow(10.0f, gain_ / 20.0f);
            song_->instruments()[instrument_index_].set_volume(v);
        }
    }
    if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0)) {
        gain_ = 0.0f;
        if(song_ && instrument_index_ < song_->instruments().size()) {
            drum::volume v = song_->instruments()[instrument_index_].volume();
            v.value = 1.0f;
            song_->instruments()[instrument_index_].set_volume(v);
        }
    }
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Remove track") && handler_) {
            handler_->emit<event::remove_track>(instrument_index_);
        }
        ImGui::EndPopup();
    }

    return true;
}

}
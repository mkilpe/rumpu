#include "track_pane.hpp"
#include "events.hpp"
#include "undo_manager.hpp"

#include "imgui.h"
#include "imgui-knobs.h"

#include <cmath>

namespace securepath::drum::app {

track_pane::track_pane(std::string name)
: child_window_base(std::move(name), ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

void track_pane::set_context(event_system::event_handler& h, song* s, std::uint32_t section, std::size_t track_index, drum::track const& t, undo_manager* undo) {
    undo_ = undo;
    handler_ = &h;
    song_ = s;
    section_ = section;
    track_index_ = track_index;
    instrument_index_ = t.instrument_index();
    if(s && instrument_index_ < s->instruments().size()) {
        auto const& instr = s->instruments()[instrument_index_];
        display_name_ = t.name().empty() ? instr.name() : t.name();
        float vol = t.volume().value;
        gain_ = 20.0f * std::log10(std::max(vol, 1e-6f));
    }
}

bool track_pane::do_draw()
{
    if (!display_name_.empty()) {
        float avail = ImGui::GetContentRegionAvail().x;
        ImVec2 text_size = ImGui::CalcTextSize(display_name_.c_str());
        bool clipped = text_size.x > avail;
        if (clipped) {
            ImGui::PushClipRect(ImGui::GetCursorScreenPos(),
                ImVec2{ImGui::GetCursorScreenPos().x + avail, ImGui::GetCursorScreenPos().y + text_size.y}, true);
        }
        ImGui::TextUnformatted(display_name_.c_str());
        if (clipped) {
            ImGui::PopClipRect();
        }
        if (clipped && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", display_name_.c_str());
        }
    }

    auto* sec = song_ ? song_->find_section(section_) : nullptr;
    drum::track* trk = (sec && track_index_ < sec->tracks().size())
        ? &sec->tracks()[track_index_] : nullptr;

    if (trk) {
        bool muted = trk->volume().mute;

        ImVec4 btn_color = muted
            ? ImVec4(0.7f, 0.2f, 0.2f, 1.0f)
            : ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
        ImVec4 btn_hovered = muted
            ? ImVec4(0.8f, 0.3f, 0.3f, 1.0f)
            : ImVec4(0.35f, 0.35f, 0.35f, 1.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, btn_color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, btn_hovered);

        std::string btn_id = "M##m" + std::to_string(track_index_);
        if (ImGui::SmallButton(btn_id.c_str())) {
            if (undo_ && song_) { undo_->snapshot(*song_); }
            drum::volume v = trk->volume();
            v.mute = !muted;
            trk->set_volume(v);
        }

        ImGui::PopStyleColor(2);
        ImGui::SameLine();
    }

    if (ImGuiKnobs::Knob("Gain", &gain_, -6.0f, 6.0f, 0.1f, "%.1f", ImGuiKnobVariant_Tick, 30)) {
        if(trk) {
            if (undo_ && song_) { undo_->snapshot(*song_, coalesce_key::track_volume); }
            drum::volume v = trk->volume();
            v.value = std::pow(10.0f, gain_ / 20.0f);
            trk->set_volume(v);
        }
    }
    if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0)) {
        if (undo_ && song_) { undo_->snapshot(*song_); }
        gain_ = 0.0f;
        if(trk) {
            drum::volume v = trk->volume();
            v.value = 1.0f;
            trk->set_volume(v);
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
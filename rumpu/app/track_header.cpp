
#include "track_list.hpp"
#include "undo_manager.hpp"

#include "imgui.h"

#include <cstdio>

namespace securepath::drum::app {

track_header::track_header()
: child_window_base("track_header", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

void track_header::set_size(const ImVec2& size)
{
    original_size_ = size;
    child_window_base::set_size(size);
}

void track_header::zoom(float z)
{
    auto size = original_size_;
    size.x *= z;
    child_window_base::set_size(size);
}

void track_header::set_context(song* s, uint32_t section, undo_manager* undo)
{
    undo_ = undo;
    song_ = s;
    section_ = section;
}

void track_header::context_menu(section* sec, ImVec2 header_pos, float lead_x, float bar_width)
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick("track_header_context", ImGuiPopupFlags_MouseButtonRight);
    }

    if (ImGui::BeginPopup("track_header_context")) {
        if (mouse_pos_.x == 0 && mouse_pos_.y == 0) {
            mouse_pos_ = io.MousePos;
        }

        // compute bar index from mouse position relative to track header
        float rel_x = mouse_pos_.x - header_pos.x - lead_x;
        uint32_t bar_index = 0;
        if (rel_x > 0 && bar_width > 0) {
            bar_index = static_cast<uint32_t>(rel_x / bar_width);
            if (bar_index >= sec->length())
                bar_index = sec->length() - 1;
        }

        auto change = sec->find_change(bar_index);
        bool has_tempo = change && change->tempo_change;

        if (has_tempo) {
            char label[64];
            std::snprintf(label, sizeof(label), "Edit Tempo (Bar %u)...", bar_index + 1);
            if (ImGui::MenuItem(label)) {
                tempo_dialog_bar_ = bar_index;
                tempo_dialog_value_ = change->tempo_change->value;
                tempo_dialog_open_ = true;
            }
            std::snprintf(label, sizeof(label), "Remove Tempo Change (Bar %u)", bar_index + 1);
            if (ImGui::MenuItem(label)) {
                if (undo_ && song_) { undo_->snapshot(*song_); }
                sec->set_tempo_change(bar_index, std::nullopt);
            }
        } else {
            char label[64];
            std::snprintf(label, sizeof(label), "Set Tempo (Bar %u)...", bar_index + 1);
            if (ImGui::MenuItem(label)) {
                tempo_dialog_bar_ = bar_index;
                tempo_dialog_value_ = song_ ? song_->default_tempo().value : 120.0f;
                tempo_dialog_open_ = true;
            }
        }

        ImGui::EndPopup();
    } else {
        mouse_pos_ = {};
    }
}

void track_header::tempo_dialog(section* sec)
{
    if (tempo_dialog_open_) {
        ImGui::OpenPopup("Set Tempo##track_header");
        tempo_dialog_open_ = false;
    }

    if (ImGui::BeginPopupModal("Set Tempo##track_header", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        char label[32];
        std::snprintf(label, sizeof(label), "Bar %u", tempo_dialog_bar_ + 1);
        ImGui::Text("%s", label);

        ImGui::InputFloat("BPM", &tempo_dialog_value_, 1.0f, 10.0f, "%.1f");
        if (tempo_dialog_value_ < 20.0f) tempo_dialog_value_ = 20.0f;
        if (tempo_dialog_value_ > 400.0f) tempo_dialog_value_ = 400.0f;

        if (ImGui::Button("OK")) {
            if (undo_ && song_) { undo_->snapshot(*song_); }
            sec->set_tempo_change(tempo_dialog_bar_, tempo{tempo_dialog_value_});
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

bool track_header::do_draw()
{
    auto drawlist = ImGui::GetWindowDrawList();

    auto pos  = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetWindowSize();

    std::size_t lead_x = 10;

    if(song_) {
        if(auto sec = song_->find_section(section_)) {

            float x = pos.x + lead_x;
            std::size_t beat_per_bar = song_->default_time_signature().beats_in_bar();
            float inc = (size.x / sec->length()) / beat_per_bar;
            float bar_width = inc * beat_per_bar;

            // invisible button for mouse interaction
            ImGui::InvisibleButton("track_header_canvas", size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

            // context menu and dialog
            context_menu(sec, pos, static_cast<float>(lead_x), bar_width);
            tempo_dialog(sec);

            // draw ticks, bar numbers, and tempo indicators
            std::size_t i = 0;
            ImVec2 tick_pos{x, pos.y};
            while(tick_pos.x < pos.x + size.x) {
                bool is_bar_start = (i % beat_per_bar == 0);
                uint32_t bar_index = static_cast<uint32_t>(i / beat_per_bar);

                auto change = is_bar_start ? sec->find_change(bar_index) : std::optional<section_bar_change>();
                bool has_tempo = change && change->tempo_change;

                auto end = tick_pos;
                auto color = IM_COL32(255,127,127,255);
                if(is_bar_start) {
                    end.y += 10;
                    color = has_tempo ? IM_COL32(255,180,50,255) : IM_COL32(100,255,100,255);
                } else {
                    end.y += 5;
                }
                ++i;
                drawlist->AddLine(tick_pos, end, color, 1.0f);

                if(is_bar_start) {
                    auto label = std::to_string(bar_index + 1);
                    auto text_size = ImGui::CalcTextSize(label.c_str());
                    if(bar_width > text_size.x) {
                        drawlist->AddText(ImVec2(tick_pos.x + 2, tick_pos.y + 10), IM_COL32(200,200,200,255), label.c_str());
                    }

                    // draw tempo change indicator
                    if(has_tempo) {
                        char bpm_label[32];
                        std::snprintf(bpm_label, sizeof(bpm_label), "%.0f", change->tempo_change->value);
                        auto bpm_size = ImGui::CalcTextSize(bpm_label);
                        if(bar_width > bpm_size.x) {
                            drawlist->AddText(ImVec2(tick_pos.x + 2, tick_pos.y + 20), IM_COL32(255,180,50,255), bpm_label);
                        }
                    }
                }

                tick_pos.x += inc;
            }
        }
    }
    return true;
}

}


#include "events.hpp"
#include "toolbar.hpp"

#include <securepath/log/log.hpp>

#include "imgui.h"

namespace securepath::drum::app {

toolbar::toolbar(event_system::event_handler& h)
: child_window_base("toolbar", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
, handler_(h)
{}

void toolbar::set_context(song* s, uint32_t section)
{
    song_ = s;
    section_ = section;
}

template<typename Event>
void toolbar::button(const std::string& label) {
    ImGui::Button(label.c_str());
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        if(song_) {
            handler_.emit<Event>(section_);
        }
    }   
}

void toolbar::set_play_status(play_status const& s)
{
    status_ = s;
}

bool toolbar::do_draw()
{
    if (ImGui::BeginTable("toolbar_table", 2, ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        button<event::open_add_track_dialog>("Add track");
        ImGui::TableNextColumn();
        ImGui::BeginGroup();
        button<event::play_song>("Play");
        ImGui::SameLine();
        button<event::stop_song>("Stop");
        {
            auto cur_ms = status_.current_time.count();
            auto tot_ms = status_.total_time.count();
            auto cur_s  = cur_ms / 1000;
            auto tot_s  = tot_ms / 1000;
            const char* name = "";
            if (song_) {
                if (auto const* sec = song_->find_section(status_.section_id)) {
                    name = sec->name().c_str();
                }
            }
            ImGui::SameLine();
            ImGui::Text("| %s | Bar %u/%u | %lld:%02lld.%03lld / %lld:%02lld.%03lld",
                name,
                status_.current_bar + 1, status_.total_bars,
                (long long)(cur_s / 60), (long long)(cur_s % 60), (long long)(cur_ms % 1000),
                (long long)(tot_s / 60), (long long)(tot_s % 60), (long long)(tot_ms % 1000));
        }
        ImGui::EndGroup();
        ImGui::EndTable();
    }
    return true;
}

}

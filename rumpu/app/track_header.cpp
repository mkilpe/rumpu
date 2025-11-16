
#include "track_list.hpp"

#include "imgui.h"

namespace securepath::drum::app {

track_header::track_header()
: child_window_base("track_header", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
{}

void track_header::set_context(song* s, uint32_t section)
{
    song_ = s;
    section_ = section;
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

            std::size_t i = 0;            
            ImVec2 tick_pos{x, pos.y};
            while(tick_pos.x < pos.x + size.x) {
                auto end = tick_pos;
                if(i++ % beat_per_bar == 0) {
                    end.y += 10;
                } else {
                    end.y += 5;
                }
                drawlist->AddLine(tick_pos, end, IM_COL32(255,127,127,255), 1.0f);
                tick_pos.x += inc;
            }
        }
    }
    return true;
}

}

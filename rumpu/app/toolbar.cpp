
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

bool toolbar::do_draw()
{
    if (ImGui::BeginTable("toolbar_table", 3, ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        button<event::add_track>("Add track");
        ImGui::TableNextColumn();
        button<event::play_song>("Play");
        ImGui::TableNextColumn();
        button<event::stop_song>("Stop");
        ImGui::EndTable();
    }
    return true;
}

}

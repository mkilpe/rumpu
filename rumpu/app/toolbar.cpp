
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

void toolbar::button(const std::string& label) {
    ImGui::Button(label.c_str());
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        LOG_TRACE("button clicked");
        if(song_) {
            handler_.emit<event::add_track>(section_);
        }
    }   
}

bool toolbar::do_draw()
{
    if (ImGui::BeginTable("toolbar_table", 1, ImGuiTableFlags_ScrollY)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        button("Add track");
 
        ImGui::EndTable();
    }
    return true;
}

}

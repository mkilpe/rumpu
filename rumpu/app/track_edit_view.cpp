
#include "track_edit_view.hpp"

#include "imgui.h"

namespace securepath::drum::app {

track_edit_view::track_edit_view(event_system::event_handler& h)
: child_window_base("track_edit_view", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
, toolbar_(h)
, track_list_("track_list")
{}

void track_edit_view::set_context(song* s, uint32_t section)
{
    toolbar_.set_context(s, section);
    track_list_.set_context(s, section);
}

bool track_edit_view::do_draw()
{
    if (ImGui::BeginTable("track_edit_view_table", 1)) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        toolbar_.do_draw();
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        track_list_.do_draw();
        ImGui::EndTable();
    }
    return true;
}

}

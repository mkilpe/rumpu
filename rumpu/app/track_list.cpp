
#include "track_list.hpp"
#include "track_pane.hpp"

#include "imgui.h"

namespace securepath::drum {

track_list::track_list()
{
	add_track();
	add_track();
	add_track();
}

void track_list::add_track()
{
	std::size_t index = tracks_.size();
	tracks_.push_back(
		track_info{
			child_window_ptr(new view_child_window(("pane " + std::to_string(index)).c_str(), view_ptr(new track_pane), ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})),
			child_window_ptr(new view_child_window(("track " + std::to_string(index)).c_str(), view_ptr(new track), ImGuiWindowFlags_NoSavedSettings, {}))
		});
	tracks_.back().pane->set_size({50, 75});
	tracks_.back().track->set_size({1500, 75});
}

bool track_list::draw()
{
	if (ImGui::BeginTable("track_list", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupScrollFreeze(1, 0);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		header_.draw();
		for(auto&& v : tracks_) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			v.pane->draw();
			ImGui::TableNextColumn();
			v.track->draw();
		}
		ImGui::EndTable();
	}
	return true;
}

}


#include "track_list.hpp"
#include "track_pane.hpp"
#include <securepath/log/log.hpp>

#include "imgui.h"

namespace securepath::drum::app {

track_list::track_list(std::string name, event_system::event_handler& h)
: child_window_base(std::move(name))
, handler_(h)
{
}

void track_list::add_track()
{
	std::size_t index = tracks_.size();
	tracks_.push_back(
		track_info{
			std::unique_ptr<track_pane>{new track_pane(("pane " + std::to_string(index)).c_str())},
			track_ptr{new track(("track " + std::to_string(index)).c_str())}
		});
	tracks_.back().pane->set_size({50, 75});
	tracks_.back().track->set_size({1500, 75});
	tracks_.back().track->zoom(zoom_);

	header_.set_size({1500, 20});
	header_.zoom(zoom_);
}

void track_list::set_track(drum::track const& t)
{
	add_track();
}

void track_list::update_tracks(song* s, uint32_t section) 
{
	 if(s) {
        if(auto sec = s->find_section(section)) {
        	LOG_TRACE("update tracks");
        	// very un-optimal, please rewrite
        	tracks_.clear();
        	for(auto&& t : sec->tracks()) {
        		set_track(t);
        	}
        }
    }
}

void track_list::set_context(song* s, uint32_t section)
{
	if(s) {
		header_.set_context(s, section);
		update_tracks(s, section);
		size_t i = 0;
		for(auto&& t : tracks_) {
			t.pane->set_context(handler_, s, i);
			t.track->set_context(i++, s, section);
		}
	}
}

bool track_list::do_draw()
{
	ImGuiIO& io = ImGui::GetIO();

	bool zoom_changed = false;
	if(io.MouseWheel) {
		zoom_changed = true;
		zoom_ += 0.05 * io.MouseWheel;
		if(zoom_ > 20.0) {
			zoom_ = 20.0;
		}
		if(zoom_ < 0.10) {
			zoom_ = 0.10;
		}		
	}

	if (ImGui::BeginTable("track_list_table", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupScrollFreeze(1, 0);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		header_.draw();
		if(zoom_changed) {
			header_.zoom(zoom_);
		}
		for(auto&& v : tracks_) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			v.pane->draw();
			ImGui::TableNextColumn();
			if(zoom_changed) {
				v.track->zoom(zoom_);
			}
			v.track->draw();
		}
		ImGui::EndTable();
	}
	return true;
}

}


#include "track_list.hpp"
#include "track_pane.hpp"
#include <securepath/log/log.hpp>
#include <algorithm>

#include "imgui.h"
#include "imgui_internal.h"

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
	tracks_.back().pane->set_size({75, 90});
	tracks_.back().track->set_size({1500, 90});
	tracks_.back().track->zoom(zoom_);

	header_.set_size({1500, 22});
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

void track_list::set_play_status(play_status const& s)
{
	play_status_ = s;
	for(auto&& v : tracks_) {
		v.track->set_playing(s.playing);
	}
}

void track_list::set_context(song* s, uint32_t section, undo_manager* undo)
{
	undo_ = undo;
	song_ = s;
	section_ = section;
	if(s) {
		header_.set_context(s, section, undo);
		update_tracks(s, section);
		if(auto sec = s->find_section(section)) {
			auto const& core_tracks = sec->tracks();
			for(std::size_t i = 0; i < tracks_.size() && i < core_tracks.size(); ++i) {
				tracks_[i].pane->set_context(handler_, s, section, i, core_tracks[i], undo);
				tracks_[i].track->set_context(i, s, section, undo);
			}
		}
	}
}

void track_list::apply_follow_scroll()
{
	if (!follow_cursor_ || !play_status_.playing || play_status_.section_id != section_) {
		return;
	}
	if (!song_) {
		return;
	}
	auto* sec = song_->find_section(section_);
	if (!sec || sec->length() == 0) {
		return;
	}

	float track_width = 1500.0f * zoom_;
	float bar_width = track_width / sec->length();
	float cursor_local_x = play_status_.current_bar * bar_width + play_status_.bar_progress * bar_width;

	float scroll_max = ImGui::GetScrollMaxX();
	float view_w = track_width - scroll_max;
	if (view_w <= 0.0f) {
		return;
	}
	float scroll_x = ImGui::GetScrollX();
	float right_margin = view_w * 0.85f;
	float pull_back_pos = view_w * 0.25f;

	float cursor_in_view = cursor_local_x - scroll_x;
	if (cursor_in_view > right_margin || cursor_in_view < 0.0f) {
		float target = cursor_local_x - pull_back_pos;
		if (target < 0.0f) {
			target = 0.0f;
		}
		ImGui::SetScrollX(target);
	}
}

void track_list::draw_play_cursor(float col_x, float col_top, float col_bottom)
{
	if (!play_status_.playing || play_status_.section_id != section_) {
		return;
	}
	if (!song_) {
		return;
	}
	auto* sec = song_->find_section(section_);
	if (!sec || sec->length() == 0) {
		return;
	}

	float lead_x = 10.0f;
	float track_width = 1500.0f * zoom_;
	float bar_width = track_width / sec->length();
	float cursor_x = col_x + lead_x + play_status_.current_bar * bar_width + play_status_.bar_progress * bar_width;

	auto* drawlist = ImGui::GetWindowDrawList();
	drawlist->PushClipRect(ImVec2{col_x, col_top}, ImVec2{col_x + 1500.0f * zoom_, col_bottom}, true);
	drawlist->AddLine(
		ImVec2{cursor_x, col_top},
		ImVec2{cursor_x, col_bottom},
		IM_COL32(255, 80, 80, 255), 1.0f);
	drawlist->PopClipRect();
}

bool track_list::do_draw()
{
	ImGuiIO& io = ImGui::GetIO();

	float max_zoom = 20.0f;
	if(song_) {
		if(auto* sec = song_->find_section(section_)) {
			max_zoom = std::max(20.0f, sec->length() / 3.0f);
		}
	}

	bool zoom_changed = false;
	// no zooming underneath an open popup/modal (e.g. the apply-pattern dialog)
	if(io.MouseWheel && ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)
		&& !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)) {
		zoom_changed = true;
		zoom_ += 0.05f * (max_zoom / 20.0f) * io.MouseWheel;
		if(zoom_ > max_zoom) {
			zoom_ = max_zoom;
		}
		if(zoom_ < 0.10) {
			zoom_ = 0.10;
		}
	}

	float col_x = 0.0f;
	float col_top = 0.0f;
	float col_bottom = 0.0f;

	{
		ImGuiContext& g = *ImGui::GetCurrentContext();
		g.NextWindowData.HasFlags |= ImGuiNextWindowDataFlags_HasWindowFlags;
		g.NextWindowData.WindowFlags = ImGuiWindowFlags_NoScrollWithMouse;
	}
	if (ImGui::BeginTable("track_list_table", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupScrollFreeze(1, 0);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TableNextColumn();
		col_x = ImGui::GetCursorScreenPos().x;
		col_top = ImGui::GetCursorScreenPos().y;
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
		col_bottom = ImGui::GetCursorScreenPos().y;
		apply_follow_scroll();
		ImGui::EndTable();
	}

	draw_play_cursor(col_x, col_top, col_bottom);

	return true;
}

}

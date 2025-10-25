#include "track.hpp"

#include <securepath/log/log.hpp>

#include "imgui.h"

namespace securepath::drum::app {

track::track(std::string name)
: track_view(std::move(name), ImGuiWindowFlags_NoSavedSettings, {})
{
}

void track::set_context(size_t index, song* s, uint32_t section)
{
	index_ = index;
	song_ = s;
	section_ = section;
}

struct drawer {	
	drawer(ImVec2 pos, ImVec2 size, time_signature sig, std::size_t bars) 
	: pos(pos)
	, size(size)
	, mark_height(size.y/3)
	, accent_height((size.y*2)/3)
	, hit_size(size.y / 12)
	, signature(sig)
	, bars(bars)
	, bar_width(size.x / bars)
	{
		hit_size = std::clamp(double(hit_size), 5.0, size.y/3.0);
	}

	void draw_mark(float x, bool accent, beat::action_type action) {
		ImVec2 p{x, pos.y+size.y/2};
		auto end = p;
		if(accent) {
			p.y -= accent_height/2;
			end.y = p.y + accent_height;
		} else {
			p.y -= mark_height/2;
			end.y = p.y + mark_height;
		}
		drawlist->AddLine(p, end, IM_COL32(150,150,150,255), 1.0f);

		p = ImVec2{x, pos.y+size.y/2};
		if(action == beat::hit) {
			drawlist->AddCircleFilled(p, hit_size, IM_COL32(225,225,225,255));
		} else if(action == beat::stop) {
			drawlist->AddCircleFilled(p, hit_size, IM_COL32(200,0,0,255));
		}		
	}

	void draw_center_line() {
		ImVec2 start{pos.x, pos.y+size.y/2};
		ImVec2 end{pos.x+size.x, pos.y+size.y/2};		
		drawlist->AddLine(start, end, IM_COL32(255,255,255,255), 1.0f);
	}

	void draw_bar(const drum::bar& bar) {
		if(bar.beats.empty()) {
			draw_default_marks();			
		} else {
			draw_marks_with_hits(bar);
		}
	}
		
	void draw_default_marks() {
		bool first = true;
		double my_x = pos.x + bar_index++*bar_width + hit_size/2.0;
		double inc = bar_width / signature.beats_in_bar();
		for(int i = 0; i != signature.beats_in_bar(); ++i) {
			LOG_TRACE("default");
			draw_mark(my_x, first, beat::none);
			first = false;
			my_x += inc;
		}
	}

	void draw_marks_with_hits(const drum::bar& bar) {
		bool first = true;
		double my_x = pos.x + bar_index++*bar_width + hit_size/2.0;
		double inc = bar_width / bar.beats.size();

		for(auto&& beat : bar.beats) {
			if(!beat.division.empty()) {
				LOG_TRACE("division");
			//	draw_split(my_x, inc, first, beat.division);				
			} else {
				LOG_TRACE("beats");
				draw_mark(my_x, first, beat.action);
			}
			first = false;
			my_x += inc;
		}
	}

	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	ImVec2 pos;
	ImVec2 size;
	std::size_t mark_height{};
	std::size_t accent_height{};
	std::size_t hit_size{};
	time_signature signature;
	std::size_t bar_index{};
	std::size_t bars{};
	double bar_width{};	
};

bool track::do_draw()
{	
	auto pos  = ImGui::GetCursorScreenPos();
	auto size = ImGui::GetWindowSize();

	if(song_) {
		if(auto sec = song_->find_section(section_)) {
			drawer d(pos, size, song_->default_time_signature(), sec->length());
			d.draw_center_line();
		
			if(auto section = song_->find_section(section_)) {
				auto tracks = section->tracks();
				if(index_ < tracks.size()) {
					auto bars = tracks[index_].bars();
					for(auto&& bar : bars) {
						d.draw_bar(bar);
					}
				}					
			}
		}
	}

	return true;
}

}
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

struct track_draw_context {
	track_draw_context(drum::song& song, size_t index, uint32_t s) {
		if(auto section = song.find_section(s)) {
			signature = song.default_time_signature();
			bar_count = section->length();
			auto& tracks = section->tracks();
			if(index < tracks.size()) {
				bars = &tracks[index].bars();
			}					
		}
	}

	operator bool() const {
		return bars != nullptr;
	}

	ImVec2 pos  = ImGui::GetCursorScreenPos();
	ImVec2 size = ImGui::GetWindowSize();
	time_signature signature;
	std::uint32_t bar_count{};
	std::deque<bar>* bars{};
};


struct drawer {	
	drawer(track_draw_context& c) 
	: pos(c.pos)
	, size(c.size)
	, mark_height(size.y/3)
	, accent_height((size.y*2)/3)
	, hit_size(size.y / 12)
	, signature(c.signature)
	, bars(c.bar_count)
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
		double my_x = pos.x + bar_index++*bar_width + lead_x;
		double inc = bar_width / signature.beats_in_bar();
		for(int i = 0; i != signature.beats_in_bar(); ++i) {
			draw_mark(my_x, first, beat::none);
			first = false;
			my_x += inc;
		}
	}

	void draw_marks_with_hits(const drum::bar& bar) {
		bool first = true;
		double my_x = pos.x + bar_index++*bar_width + lead_x;
		double inc = bar_width / bar.beats.size();

		for(auto&& beat : bar.beats) {
			if(!beat.division.empty()) {
			//	draw_split(my_x, inc, first, beat.division);				
			} else {
				draw_mark(my_x, first, beat.action);
			}
			first = false;
			my_x += inc;
		}
	}

	ImDrawList* drawlist = ImGui::GetWindowDrawList();
	ImVec2 pos;
	ImVec2 size;
	std::size_t lead_x{10};
	std::size_t mark_height{};
	std::size_t accent_height{};
	std::size_t hit_size{};
	time_signature signature;
	std::size_t bar_index{};
	std::size_t bars{};
	double bar_width{};	
};

void track::toggle_mark(track_draw_context& context, const ImVec2& rel_pos) {
	std::size_t lead_x{10};
	float bar_width = context.size.x / context.bar_count;

	auto start_x = ImGui::GetScrollX();
	auto content_x = start_x + rel_pos.x;

	content_x -= lead_x;
	std::size_t low_index = content_x / bar_width;
	std::size_t high_index = low_index+1;
	
	if(low_index < context.bars->size()) {
		auto& beats = (*context.bars)[low_index].beats;
		auto inner_pos = content_x - bar_width*low_index;		
		float inner_factor = inner_pos / bar_width;
		std::size_t beat_count = beats.empty() ? context.signature.beats_in_bar() : beats.size();
		std::size_t inner_index = std::round(inner_factor * beat_count);
		
		beat* b{};
		if(inner_index >= beats.size() && high_index < context.bars->size()) {
			auto& high_beats = (*context.bars)[high_index].beats;
			if(high_beats.empty()) {
				high_beats.push_back(beat{beat::none});
			}
			b = &high_beats[0];			
		} else {
			if(beats.size() <= inner_index) {
				beats.resize(inner_index+1);
			}
			b = &beats[inner_index];
		}
		b->action = b->action == beat::none ? beat::hit : beat::none;
	}
}

void track::handle_mouse(track_draw_context& context) {
	ImGuiIO& io = ImGui::GetIO();
	
	ImGui::InvisibleButton((name()+"_mouse_canvas").c_str(), context.size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool hovered = ImGui::IsItemHovered();
    //bool active = ImGui::IsItemActive();
    
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
    	toggle_mark(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y});
    }

    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    auto context_name = name() + "_context";
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick(context_name.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }
    if (ImGui::BeginPopup(context_name.c_str())) {        
        ImGui::EndPopup();
    }
}

bool track::do_draw()
{	
	if(song_) {
		if(track_draw_context context{*song_, index_, section_}) {
			// mouse interaction
			handle_mouse(context);

			drawer d(context);
			d.draw_center_line();
		
			for(auto&& bar : *context.bars) {
				d.draw_bar(bar);
			}
		}
	}

	return true;
}

}
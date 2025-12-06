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

struct bar_calc {
	bar_calc(track_draw_context& context, const ImVec2& rel_pos)
	: context(context)
	, bar_width(context.size.x / context.bar_count)
	, content_x(ImGui::GetScrollX() + rel_pos.x - lead_x)
	, index(content_x / bar_width)
	{
	}

	bar* find_bar() const {
		return index < context.bars->size() ? &(*context.bars)[index] : nullptr;
	}

	struct beat_index {
		std::size_t bar_index{};
		float index_pos{};
	};

	beat* find_beat_impl(beat_index bi) const {
		beat* prev{};
		float prev_pos{};
		while(bi.bar_index < context.bar_count) {
			auto& beats = (*context.bars)[bi.bar_index].beats;
			if(!beats.empty()) {
				float beat_pos_inc = bar_width / beats.size();
				auto pos = bi.index_pos;
				for(auto&& b : beats) {
					if(b.division.empty()) {
						if(pos >= x_content) {	
							return !prev || (pos - x_content) >= (x_content - prev_pos) ? &b : prev;
						}
					} else {
						auto fb = find_beat_div(b.division);
						if()
					}
					pos += beat_pos_inc;
					prev = &b;
					prev_pos = pos;
				}
			}			
			++bi.bar_index;
			bi.index_pos += bar_width;
		}
	}

	beat* find_beat() const {
		beat* res{};
		if(index < context.bars->size()) {
			a
			if(!beats.empty()) {
				res = find_beat_impl(beat_index{index, bar_width*index});
			}
		}
		return res;
	}


	void toggle_mark() const {
		if(beat* b = find_beat()) {
			b->action = b->action == beat::none ? beat::hit : beat::none;
		}
	}

	track_draw_context& context;
	std::size_t lead_x{10};
	float bar_width;
	float content_x;
	std::size_t index;
};

void track::context_menu(track_draw_context& context)
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    auto context_name = name() + "_context";
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick(context_name.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }
    if (ImGui::BeginPopup(context_name.c_str())) {
    	if (ImGui::MenuItem("Split beat")) {
    		bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y});
    		if(auto b = bc.find_beat()) {

    		}
    	}
		if (ImGui::MenuItem("Divide beat")) {
    		
    	}
		if (ImGui::MenuItem("Divide bar")) {
    		bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y});
    		if(auto b = bc.find_bar()) {
    			b->beats.resize(7);
    		}
    	}
    	if (ImGui::MenuItem("Clear bar")) {
    		bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y});
    		if(auto b = bc.find_bar()) {
    			for(auto&& beat : b->beats) {
    				beat.action = beat::none;
    				beat.division.clear();
    				//clear other data too?
    			}
    		}
    	}

        ImGui::EndPopup();
    }
}

void track::handle_mouse(track_draw_context& context) {
	ImGuiIO& io = ImGui::GetIO();
	
	ImGui::InvisibleButton((name()+"_mouse_canvas").c_str(), context.size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool hovered = ImGui::IsItemHovered();
    
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
    	bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y});
    	bc.toggle_mark();
    }

    context_menu(context);
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
#include "track.hpp"
#include <rumpu/core/undo_manager.hpp>

#include <securepath/log/log.hpp>

#include "imgui.h"

#include <print>

namespace securepath::drum::app {

track::track(std::string name)
: track_view(std::move(name), ImGuiWindowFlags_NoSavedSettings, {})
{
}

void track::set_context(size_t index, song* s, uint32_t section, undo_manager* undo)
{
	undo_ = undo;
	index_ = index;
	song_ = s;
	section_ = section;
	beat_props_beat_ = nullptr;
}

struct track_draw_context {
	track_draw_context(drum::song& song, size_t index, uint32_t s)
	: song_(&song)
	{
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

	drum::song* song_{};
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
		ImVec2 p{std::round(x), pos.y+size.y/2};
		auto end = p;
		if(accent) {
			p.y -= accent_height/2;
			end.y = p.y + accent_height;
		} else {
			p.y -= mark_height/2;
			end.y = p.y + mark_height;
		}
		drawlist->AddLine(p, end, tick_color, 1.0f);

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

	void draw_split(double x, int w, bool accent, std::vector<beat> const& div) {
		auto old_color = tick_color;
		tick_color = IM_COL32(150,150,210,255);
		draw_marks(x, w, accent, div);
		tick_color = old_color;
	}

	void draw_marks(double x, int width, bool accent, std::vector<beat> const& beats) {
		double inc = double(width) / beats.size();
		for(auto const& beat : beats) {
			if(!beat.division.empty()) {
				draw_split(x, inc, accent, beat.division);				
			} else {
				draw_mark(x, accent, beat.action);
			}			
			if(accent) {
				accent = false;
			}
			x += inc;
		}
	}

	void draw_marks_with_hits(const drum::bar& bar) {
		double my_x = pos.x + bar_index++*bar_width + lead_x;		
		draw_marks(my_x, bar_width, true, bar.beats);		
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
	ImU32 tick_color{IM_COL32(150,150,150,255)};
};

struct bar_calc {
	bar_calc(track_draw_context& context, const ImVec2& rel_pos, bool closest_beat = false)
	: context(context)
	, bar_width(context.size.x / context.bar_count)
	, content_x(ImGui::GetScrollX() + rel_pos.x - lead_x)
	, index(content_x / bar_width)
	, closest_beat(closest_beat)
	{
	}

	bar* find_bar() const {
		return index < context.bars->size() ? &(*context.bars)[index] : nullptr;
	}

	struct beat_index {
		std::size_t bar_index{};
		float index_pos{};
	};

	beat* find_beat_impl(float pos, float width, std::vector<beat>& beats, beat*& prev, float& prev_pos) const {
		float beat_pos_inc = width / beats.size();
		for(auto&& b : beats) {
			if(b.division.empty()) {
				if(pos >= content_x) {
					//std::println("pos = {}, prev_pos = {}, content_x = {}", pos, prev_pos, content_x);
					if(closest_beat) {
						return !prev || (pos - content_x) < (content_x - prev_pos) ? &b : prev;
					}
					return prev ? prev : &b;
				}
				prev_pos = pos;
				prev = &b;
			} else {				
				if(auto res = find_beat_impl(pos, beat_pos_inc, b.division, prev, prev_pos)) {
					return res;
				}
			}
			pos += beat_pos_inc;
		}
		return nullptr;
	}

	beat* find_beat_impl(beat_index bi) const {
		beat* prev{};
		float prev_pos{};
		while(bi.bar_index < context.bar_count) {
			auto& beats = (*context.bars)[bi.bar_index].beats;
			if(!beats.empty()) {
				if(auto res = find_beat_impl(bi.index_pos, bar_width, beats, prev, prev_pos)) {
					return res;
				}
			}			
			++bi.bar_index;
			bi.index_pos += bar_width;
		}
		return prev;
	}

	beat* find_beat() const {
		beat* res{};
		if(index < context.bars->size()) {			
			res = find_beat_impl(beat_index{index, bar_width*index});
		}
		return res;
	}


	void toggle_mark() const {
		if(index < context.bars->size()) {
			auto& bar = (*context.bars)[index];
			if(bar.beats.empty()) {
				bar.beats.resize(context.signature.beats_in_bar());
			}
		}
		if(beat* b = find_beat()) {
			if(b->action == beat::none) {
				b->action = beat::hit;
				context.song_->randomise_beat(*b);
			} else {
				b->action = beat::none;
				b->stop_data.falloff = nullptr;
			}
		}
	}

	track_draw_context& context;
	std::size_t lead_x{10};
	float bar_width;
	float content_x;
	std::size_t index;
	bool closest_beat{false};
};

void track::context_menu(track_draw_context& context)
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    auto context_name = name() + "_context";
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick(context_name.c_str(), ImGuiPopupFlags_MouseButtonRight);
        // remember where the mouse was when opening popup        
    }

    if (ImGui::BeginPopup(context_name.c_str())) {
    	if (mouse_pos_.x == 0 && mouse_pos_.y == 0) {
    		mouse_pos_ = io.MousePos;
    	}
    	if (ImGui::MenuItem("Split beat")) {
    		if (undo_ && song_) { undo_->snapshot(*song_); }
    		bar_calc bc(context, ImVec2{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y});
    		if(auto b = bc.find_beat()) {
				b->division.resize(2);
    			b->division.front() = b->data();
    		}
    	}
		//if (ImGui::MenuItem("Divide beat")) {
		if (ImGui::BeginMenu("Divide beat"))
        {
        	std::size_t amount = 0;
            if(ImGui::MenuItem("2")) {
				amount = 2;
            }
            if(ImGui::MenuItem("3")) {
				amount = 3;
            }
            if(ImGui::MenuItem("4")) {
				amount = 4;
            }
            if(ImGui::MenuItem("5")) {
				amount = 5;
            }
            if(ImGui::MenuItem("...")) {
            	divide_dialog_open_ = true;
            	divide_dialog_target_ = divide_target::beat;
            	divide_amount_ = 2;
            	divide_mouse_pos_ = mouse_pos_;
            }
            if(amount) {
            	if (undo_ && song_) { undo_->snapshot(*song_); }
    			bar_calc bc(context, ImVec2{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y});
    			if(auto b = bc.find_beat()) {
				    b->division.resize(amount);
				    b->division.front() = b->data();
    			}
    		}
    		ImGui::EndMenu();
    	}
		if (ImGui::BeginMenu("Divide bar"))
        {
        	std::size_t amount = 0;
            if(ImGui::MenuItem("2")) {
				amount = 2;
            }
            if(ImGui::MenuItem("3")) {
				amount = 3;
            }
            if(ImGui::MenuItem("4")) {
				amount = 4;
            }
            if(ImGui::MenuItem("5")) {
				amount = 5;
            }
            if(ImGui::MenuItem("...")) {
            	divide_dialog_open_ = true;
            	divide_dialog_target_ = divide_target::bar;
            	divide_amount_ = 2;
            	divide_mouse_pos_ = mouse_pos_;
            }
            if(amount) {
            	if (undo_ && song_) { undo_->snapshot(*song_); }
    			bar_calc bc(context, ImVec2{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y});
    			if(auto b = bc.find_bar()) {
    				b->beats.resize(amount);
    			}
    		}
            ImGui::EndMenu();
    	}
    	if (ImGui::MenuItem("Clear bar")) {
    		if (undo_ && song_) { undo_->snapshot(*song_); }
    		bar_calc bc(context, ImVec2{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y});
    		if(auto b = bc.find_bar()) {
    			for(auto&& beat : b->beats) {
    				beat.action = beat::none;
    				beat.division.clear();
    				//clear other data too?
    			}
    		}
    	}
		{
			bar_calc bc(context, ImVec2{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y});
			auto b = bc.find_beat();
			bool has_none = b && b->action == beat::none;
			bool has_hit = b && b->action == beat::hit;
			bool has_stop = b && b->action == beat::stop;

			if (ImGui::MenuItem("Set choke", nullptr, false, has_none)) {
				if (undo_ && song_) { undo_->snapshot(*song_); }
				if(b) {
					b->action = beat::stop;
					if(!b->stop_data.falloff) {
						b->stop_data.falloff = audio_falloff::create(audio_falloff::exponential);
					}
				}
			}
			if (ImGui::MenuItem("Remove choke", nullptr, false, has_stop)) {
				if (undo_ && song_) { undo_->snapshot(*song_); }
				if(b) {
					b->action = beat::none;
					b->stop_data.falloff = nullptr;
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem(has_stop ? "Choke properties..." : "Beat properties...", nullptr, false, has_hit || has_stop)) {
				if(b) {
					beat_props_beat_ = b;
					beat_props_open_ = true;
					beat_props_mouse_pos_ = mouse_pos_;
				}
			}
		}

        ImGui::EndPopup();
    } else {
    	mouse_pos_ = {};
    }
}

void track::handle_mouse(track_draw_context& context) {
	ImGuiIO& io = ImGui::GetIO();
	
	ImGui::InvisibleButton((name()+"_mouse_canvas").c_str(), context.size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool hovered = ImGui::IsItemHovered();
    
	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
    	if (undo_ && song_) { undo_->snapshot(*song_); }
    	bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y}, true);
    	bc.toggle_mark();
    }

    context_menu(context);
}

void track::set_size(const ImVec2& size) 
{
	original_size_ = size;
	child_window_base::set_size(size);
}

void track::zoom(float z)
{
	zoom_ = z;
	auto size = original_size_;
	size.x *= zoom_;
	child_window_base::set_size(size);
}

void track::divide_dialog(track_draw_context& context)
{
	auto popup_name = "Divide##" + name();

	if(divide_dialog_open_) {
		ImGui::OpenPopup(popup_name.c_str());
		divide_dialog_open_ = false;
	}

	if(ImGui::BeginPopupModal(popup_name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		bool const is_beat = divide_dialog_target_ == divide_target::beat;
		ImGui::Text("Divide %s into:", is_beat ? "beat" : "bar");
		ImGui::SliderInt("##n", &divide_amount_, 2, 16);

		if(ImGui::Button("OK")) {
			if (undo_ && song_) { undo_->snapshot(*song_); }
			bar_calc bc(context, ImVec2{divide_mouse_pos_.x - context.pos.x, divide_mouse_pos_.y - context.pos.y});
			if(is_beat) {
				if(auto b = bc.find_beat()) {
					b->division.resize(divide_amount_);
					b->division.front() = b->data();
				}
			} else {
				if(auto b = bc.find_bar()) {
					b->beats.resize(divide_amount_);
				}
			}
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void track::beat_properties_dialog(track_draw_context& context)
{
	auto popup_name = "Beat Properties##" + name();

	if(beat_props_open_) {
		ImGui::OpenPopup(popup_name.c_str());
		beat_props_open_ = false;
	}

	if(ImGui::BeginPopupModal(popup_name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		if(beat_props_beat_) {
			if(beat_props_beat_->action == beat::hit) {
				auto& hd = beat_props_beat_->hit_data;
				float volume = hd.volume.value;
				float rand_offset = hd.rand_hit_offset;
				float rand_vol = hd.rand_volume.value * 100.0f;

				ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f");
				ImGui::SliderFloat("Rand offset (ms)", &rand_offset, -100.0f, 100.0f, "%.1f");
				ImGui::SliderFloat("Rand volume (%)", &rand_vol, -100.0f, 100.0f, "%.1f");

				if (volume != hd.volume.value || rand_offset != hd.rand_hit_offset || rand_vol != hd.rand_volume.value * 100.0f) {
					if (undo_ && song_) { undo_->snapshot(*song_, coalesce_key::beat_property); }
				}
				hd.volume.value = volume;
				hd.rand_hit_offset = rand_offset;
				hd.rand_volume.value = rand_vol / 100.0f;
			} else if(beat_props_beat_->action == beat::stop) {
				auto& sd = beat_props_beat_->stop_data;
				if(!sd.falloff) {
					sd.falloff = audio_falloff::create(audio_falloff::exponential);
				}
				int type = static_cast<int>(sd.falloff->type());
				float duration = sd.falloff->duration_beats();

				ImGui::SeparatorText("Falloff");
				ImGui::RadioButton("Immediate", &type, audio_falloff::immediate);
				ImGui::RadioButton("Linear", &type, audio_falloff::linear);
				ImGui::RadioButton("Exponential", &type, audio_falloff::exponential);

				bool is_immediate = (type == audio_falloff::immediate);
				ImGui::BeginDisabled(is_immediate);
				ImGui::SliderFloat("Duration (beats)", &duration, 0.1f, 8.0f, "%.1f");
				ImGui::EndDisabled();

				if(type != static_cast<int>(sd.falloff->type()) || duration != sd.falloff->duration_beats()) {
					if (undo_ && song_) { undo_->snapshot(*song_, coalesce_key::beat_property); }
					sd.falloff = audio_falloff::create(static_cast<audio_falloff::falloff_type>(type), duration);
				}
			}
		}

		if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			beat_props_beat_ = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

bool track::do_draw()
{
	if(song_) {
		if(track_draw_context context{*song_, index_, section_}) {
			// mouse interaction
			handle_mouse(context);
			divide_dialog(context);
			beat_properties_dialog(context);

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
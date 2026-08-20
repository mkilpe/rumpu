#include "track.hpp"
#include <rumpu/core/undo_manager.hpp>
#include <rumpu/core/song_edit.hpp>

#include <securepath/log/log.hpp>

#include "imgui.h"

#include <algorithm>
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
	track_draw_context() = default;
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
	// captured at construction, inside the window that owns the canvas: popup
	// code runs in a different window whose ambient GetScrollX() is useless
	float scroll_x = ImGui::GetScrollX();
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
		// absolute cap: the circle marks a position, so on tall canvases
		// (apply-pattern dialog, tall track rows) it should stop growing
		hit_size = std::clamp(double(hit_size), 5.0, 12.0);
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
	, content_x(context.scroll_x + rel_pos.x - lead_x)
	// a click in the lead-in margin gives a negative position; map it past the
	// last bar instead of converting a negative float to size_t (UB)
	, index(content_x < 0 ? std::numeric_limits<std::size_t>::max()
		: static_cast<std::size_t>(content_x / bar_width))
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


	// true when toggle_mark() would actually change the song; lets the caller
	// take the undo snapshot only for real edits
	bool would_toggle() const {
		if(index >= context.bars->size()) {
			return false;
		}
		return (*context.bars)[index].beats.empty() || find_beat() != nullptr;
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

// menu actions shared by the track context menu and the pattern editor menu;
// callers that edit the song wrap these in a song_edit

static std::size_t amount_menu_items(std::initializer_list<std::size_t> amounts) {
	std::size_t amount = 0;
	for(auto a : amounts) {
		if(ImGui::MenuItem(std::to_string(a).c_str())) {
			amount = a;
		}
	}
	return amount;
}

static void split_beat_at(track_draw_context& ctx, ImVec2 rel) {
	bar_calc bc(ctx, rel);
	if(auto b = bc.find_beat()) {
		b->division.resize(2);
		b->division.front() = b->data();
	}
}

static void divide_beat_at(track_draw_context& ctx, ImVec2 rel, std::size_t amount) {
	bar_calc bc(ctx, rel);
	if(auto b = bc.find_beat()) {
		b->division.resize(amount);
		b->division.front() = b->data();
	}
}

static void divide_bar_at(track_draw_context& ctx, ImVec2 rel, std::size_t amount) {
	bar_calc bc(ctx, rel);
	if(auto b = bc.find_bar()) {
		b->beats.resize(amount);
	}
}

static void clear_bar_at(track_draw_context& ctx, ImVec2 rel) {
	bar_calc bc(ctx, rel);
	if(auto b = bc.find_bar()) {
		for(auto&& beat : b->beats) {
			beat.action = beat::none;
			beat.division.clear();
		}
	}
}

static void set_choke(beat& b) {
	b.action = beat::stop;
	if(!b.stop_data.falloff) {
		b.stop_data.falloff = audio_falloff::create(audio_falloff::exponential);
	}
}

static void remove_choke(beat& b) {
	b.action = beat::none;
	b.stop_data.falloff = nullptr;
}

void track::divide_submenu(track_draw_context& context, ImVec2 rel, char const* label, divide_target target)
{
	if (ImGui::BeginMenu(label)) {
		std::size_t const amount = amount_menu_items({2, 3, 4, 5});
		if (ImGui::MenuItem("...")) {
			divide_dialog_open_ = true;
			divide_dialog_target_ = target;
			divide_amount_ = 2;
			divide_mouse_pos_ = mouse_pos_;
		}
		if (amount) {
			song_edit edit{*song_, undo_};
			if (target == divide_target::beat) {
				divide_beat_at(context, rel, amount);
			} else {
				divide_bar_at(context, rel, amount);
			}
		}
		ImGui::EndMenu();
	}
}

void track::beat_menu_items(track_draw_context& context, ImVec2 rel)
{
	if (ImGui::MenuItem("Split beat")) {
		song_edit edit{*song_, undo_};
		split_beat_at(context, rel);
	}
	divide_submenu(context, rel, "Divide beat", divide_target::beat);
	divide_submenu(context, rel, "Divide bar", divide_target::bar);
	if (ImGui::MenuItem("Clear bar")) {
		song_edit edit{*song_, undo_};
		clear_bar_at(context, rel);
	}
}

void track::track_menu_items(track_draw_context& context, ImVec2 rel)
{
	if (ImGui::MenuItem("Clear track")) {
		song_edit edit{*song_, undo_};
		if(context.bars) {
			for(auto&& bar : *context.bars) {
				for(auto&& beat : bar.beats) {
					beat.action = beat::none;
					beat.division.clear();
				}
			}
		}
	}
	// disabled while playing: the follow-cursor section switch rebuilds the
	// track views, which would pull this dialog's state out from under it
	if (ImGui::MenuItem("Apply pattern...", nullptr, false, !playing_)) {
		apply_pattern_open_ = true;
		if(apply_pattern_bars_.empty()) {
			seed_apply_pattern(context, rel);
		}
	}
}

// one empty bar matching the signature: the pattern the dialog starts from
static bar default_pattern_bar(time_signature const& signature)
{
	bar seed;
	seed.beats.assign(signature.beats_in_bar(), beat{});
	return seed;
}

void track::seed_apply_pattern(track_draw_context& context, ImVec2 rel)
{
	bar seed;
	bar_calc bc(context, rel);
	if(auto b = bc.find_bar(); b && !b->beats.empty()) {
		seed = *b;
		for(auto&& beat : seed.beats) {
			beat.division.clear();
		}
	} else {
		seed = default_pattern_bar(context.signature);
	}
	apply_pattern_bars_.assign(1, seed);
}

void track::choke_menu_items(track_draw_context& context, ImVec2 rel)
{
	bar_calc bc(context, rel);
	auto b = bc.find_beat();
	bool const has_none = b && b->action == beat::none;
	bool const has_hit = b && b->action == beat::hit;
	bool const has_stop = b && b->action == beat::stop;

	if (ImGui::MenuItem("Set choke", nullptr, false, has_none)) {
		song_edit edit{*song_, undo_};
		set_choke(*b);
	}
	if (ImGui::MenuItem("Remove choke", nullptr, false, has_stop)) {
		song_edit edit{*song_, undo_};
		remove_choke(*b);
	}
	ImGui::Separator();
	if (ImGui::MenuItem(has_stop ? "Choke properties..." : "Beat properties...", nullptr, false, has_hit || has_stop)) {
		beat_props_beat_ = b;
		beat_props_open_ = true;
		beat_props_mouse_pos_ = mouse_pos_;
	}
}

void track::context_menu(track_draw_context& context)
{
	ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    auto context_name = name() + "_context";
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f) {
        ImGui::OpenPopupOnItemClick(context_name.c_str(), ImGuiPopupFlags_MouseButtonRight);
    }

    if (ImGui::BeginPopup(context_name.c_str())) {
    	// remember where the mouse was when opening the popup
    	if (mouse_pos_.x == 0 && mouse_pos_.y == 0) {
    		mouse_pos_ = ImGui::GetIO().MousePos;
    	}
    	ImVec2 const rel{mouse_pos_.x - context.pos.x, mouse_pos_.y - context.pos.y};
    	beat_menu_items(context, rel);
    	ImGui::Separator();
    	track_menu_items(context, rel);
    	ImGui::Separator();
    	choke_menu_items(context, rel);
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
    	bar_calc bc(context, ImVec2{io.MousePos.x - context.pos.x, io.MousePos.y - context.pos.y}, true);
    	if (bc.would_toggle()) {
    		song_edit edit{*song_, undo_};
    		bc.toggle_mark();
    	}
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
			song_edit edit{*song_, undo_};
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
				hit_beat_properties(*beat_props_beat_);
			} else if(beat_props_beat_->action == beat::stop) {
				choke_beat_properties(*beat_props_beat_);
			}
		}

		if(ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
			beat_props_beat_ = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void track::hit_beat_properties(beat& b)
{
	auto& hd = b.hit_data;
	float volume = hd.volume.value;
	float rand_offset = hd.rand_hit_offset;
	float rand_vol = hd.rand_volume.value * 100.0f;

	ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f, "%.2f");
	ImGui::SliderFloat("Rand offset (ms)", &rand_offset, -100.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Rand volume (%)", &rand_vol, -100.0f, 100.0f, "%.1f");

	bool const changed = volume != hd.volume.value || rand_offset != hd.rand_hit_offset || rand_vol != hd.rand_volume.value * 100.0f;
	song_edit edit{*song_, changed ? undo_ : nullptr, coalesce_key::beat_property};
	hd.volume.value = volume;
	hd.rand_hit_offset = rand_offset;
	hd.rand_volume.value = rand_vol / 100.0f;
}

void track::choke_beat_properties(beat& b)
{
	auto& sd = b.stop_data;
	if(!sd.falloff) {
		song_edit edit{*song_};
		sd.falloff = audio_falloff::create(audio_falloff::exponential);
	}
	int type = static_cast<int>(sd.falloff->type());
	float duration = sd.falloff->duration_beats();

	ImGui::SeparatorText("Falloff");
	ImGui::RadioButton("Immediate", &type, audio_falloff::immediate);
	ImGui::RadioButton("Linear", &type, audio_falloff::linear);
	ImGui::RadioButton("Exponential", &type, audio_falloff::exponential);

	ImGui::BeginDisabled(type == audio_falloff::immediate);
	ImGui::SliderFloat("Duration (beats)", &duration, 0.1f, 8.0f, "%.1f");
	ImGui::EndDisabled();

	if(type != static_cast<int>(sd.falloff->type()) || duration != sd.falloff->duration_beats()) {
		song_edit edit{*song_, undo_, coalesce_key::beat_property};
		sd.falloff = audio_falloff::create(static_cast<audio_falloff::falloff_type>(type), duration);
	}
}

static void pattern_context_menu_items(track_draw_context& ctx, ImVec2 rel)
{
	// edits the local pattern buffer, not the song: no song_edit wrapping
	if(ImGui::MenuItem("Split beat")) {
		split_beat_at(ctx, rel);
	}
	if(ImGui::BeginMenu("Divide beat")) {
		if(std::size_t const amount = amount_menu_items({2, 3, 4, 5})) {
			divide_beat_at(ctx, rel, amount);
		}
		ImGui::EndMenu();
	}
	if(ImGui::BeginMenu("Divide bar")) {
		if(std::size_t const amount = amount_menu_items({2, 3, 4, 5, 8, 16})) {
			divide_bar_at(ctx, rel, amount);
		}
		ImGui::EndMenu();
	}
	if(ImGui::MenuItem("Clear bar")) {
		clear_bar_at(ctx, rel);
	}
	ImGui::Separator();

	bar_calc bc(ctx, rel);
	auto b = bc.find_beat();
	bool const has_none = b && b->action == beat::none;
	bool const has_stop = b && b->action == beat::stop;
	if(ImGui::MenuItem("Set choke", nullptr, false, has_none)) {
		set_choke(*b);
	}
	if(ImGui::MenuItem("Remove choke", nullptr, false, has_stop)) {
		remove_choke(*b);
	}
}

void track::apply_pattern_dialog(track_draw_context& context)
{
	auto popup_name = "Apply pattern##" + name();

	if(apply_pattern_open_) {
		ImGui::OpenPopup(popup_name.c_str());
		apply_pattern_open_ = false;
	}

	ImGui::SetNextWindowSize(ImVec2{520.0f, 220.0f}, ImGuiCond_Appearing);
	if(ImGui::BeginPopupModal(popup_name.c_str(), nullptr, ImGuiWindowFlags_None)) {
		// the popup can outlive this object (track views are rebuilt on every
		// set_context while ImGui keeps the popup open by name) — reseed the
		// pattern instead of running the dialog on an empty vector
		if(apply_pattern_bars_.empty()) {
			apply_pattern_bars_.assign(1, default_pattern_bar(context.signature));
		}
		ImGui::TextUnformatted("Left click: toggle hit.  Right click: more actions (divide, choke...).");
		ImGui::TextUnformatted("Pattern is tiled across the whole track.");

		apply_pattern_bars_slider(context);
		apply_pattern_canvas();
		apply_pattern_footer(context);
		ImGui::EndPopup();
	}
}

void track::apply_pattern_bars_slider(track_draw_context const& context)
{
	int bar_count = std::clamp(static_cast<int>(apply_pattern_bars_.size()), 1, 4);
	if(ImGui::SliderInt("Bars", &bar_count, 1, 4)) {
		int const old = static_cast<int>(apply_pattern_bars_.size());
		apply_pattern_bars_.resize(bar_count);
		std::size_t const beats_per_bar = (old > 0 && !apply_pattern_bars_[0].beats.empty())
			? apply_pattern_bars_[0].beats.size()
			: context.signature.beats_in_bar();
		for(int i = old; i < bar_count; ++i) {
			apply_pattern_bars_[i].beats.assign(beats_per_bar, beat{});
		}
	}
}

void track::apply_pattern_canvas()
{
	float const footer_h = ImGui::GetFrameHeightWithSpacing();
	ImGui::BeginChild("##pattern_scroll", ImVec2{0.0f, -footer_h}, true,
		ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	// wheel zooms the pattern view, matching the track area behaviour
	if(ImGuiIO& io = ImGui::GetIO(); io.MouseWheel && ImGui::IsWindowHovered()) {
		apply_pattern_zoom_ = std::clamp(apply_pattern_zoom_ + 0.05f * io.MouseWheel, 0.25f, 4.0f);
	}
	float const bar_px = 320.0f * apply_pattern_zoom_;
	float const track_height = std::max(40.0f,
		ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ScrollbarSize);

	track_draw_context pctx;
	pctx.song_ = song_;
	pctx.signature = song_ ? song_->default_time_signature() : time_signature{};
	pctx.bar_count = static_cast<std::uint32_t>(apply_pattern_bars_.size());
	pctx.bars = &apply_pattern_bars_;
	pctx.pos = ImGui::GetCursorScreenPos();
	pctx.size = ImVec2{bar_px * apply_pattern_bars_.size(), track_height};

	ImGui::InvisibleButton("##pattern_canvas", pctx.size,
		ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	bool const hovered = ImGui::IsItemHovered();

	ImGui::GetWindowDrawList()->AddRectFilled(pctx.pos,
		ImVec2{pctx.pos.x + pctx.size.x, pctx.pos.y + track_height}, IM_COL32(25, 25, 25, 255));
	{
		drawer d(pctx);
		d.draw_center_line();
		for(auto&& b : apply_pattern_bars_) {
			d.draw_bar(b);
		}
	}

	apply_pattern_canvas_input(pctx, hovered);
	ImGui::EndChild();
}

void track::apply_pattern_canvas_input(track_draw_context& pctx, bool hovered)
{
	// pctx.pos is the (scroll-adjusted) content origin, so mouse - pos is a
	// content coordinate already; pre-subtract the captured scroll that
	// bar_calc adds back, popup or not
	auto make_rel = [&](ImVec2 mouse) {
		return ImVec2{mouse.x - pctx.pos.x - pctx.scroll_x, mouse.y - pctx.pos.y};
	};
	if(hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
		bar_calc bc(pctx, make_rel(ImGui::GetIO().MousePos), true);
		bc.toggle_mark();
	}
	if(hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		apply_pattern_mouse_pos_ = ImGui::GetIO().MousePos;
		ImGui::OpenPopup("##apply_pattern_menu");
	}
	if(ImGui::BeginPopup("##apply_pattern_menu")) {
		pattern_context_menu_items(pctx, make_rel(apply_pattern_mouse_pos_));
		ImGui::EndPopup();
	}
}

void track::apply_pattern_footer(track_draw_context& context)
{
	bool const has_bars = context.bars && context.bar_count > 0 && !apply_pattern_bars_.empty();
	ImGui::BeginDisabled(!has_bars);
	if(ImGui::Button("OK")) {
		apply_pattern_to_track(context);
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if(ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
		ImGui::CloseCurrentPopup();
	}
}

void track::apply_pattern_to_track(track_draw_context& context)
{
	song_edit edit{*song_, undo_};
	auto& dst_bars = *context.bars;
	for(std::size_t i = 0; i < dst_bars.size(); ++i) {
		auto const& src = apply_pattern_bars_[i % apply_pattern_bars_.size()];
		dst_bars[i].beats = src.beats;
		for(auto&& b : dst_bars[i].beats) {
			if(b.action == beat::hit) {
				context.song_->randomise_beat(b);
			} else if(b.action == beat::stop && !b.stop_data.falloff) {
				b.stop_data.falloff = audio_falloff::create(audio_falloff::exponential);
			}
		}
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
			apply_pattern_dialog(context);

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
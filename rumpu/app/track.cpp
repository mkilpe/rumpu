#include "track.hpp"

#include "imgui.h"

namespace securepath::drum::app {

track::track(std::string name)
: track_view(std::move(name), ImGuiWindowFlags_NoSavedSettings, {})
{
}

void track::set_context(track_context context)
{
	context_ = context;
}

//uto const& bars = section_->tracks()[index.row()].bars();
//			time_signature sig = song_->default_time_signature();

bool track::do_draw()
{
	auto drawlist = ImGui::GetWindowDrawList();

	auto pos  = ImGui::GetCursorScreenPos();
	auto size = ImGui::GetWindowSize();

	auto start = pos;
	start.y += size.y/2;

	auto end = start;
	end.x += size.x;

	drawlist->AddLine(start, end, IM_COL32(255,255,255,255), 1.0f);

	auto tick_pos = start;
	tick_pos.y -= 4;
	while(tick_pos.x < end.x) {
		auto end = tick_pos;
		end.y += 8;
		drawlist->AddLine(tick_pos, end, IM_COL32(255,255,255,255), 1.0f);
		tick_pos.x += 80;
	}

	if(context_.is_valid()) {
		auto const& bars = context_.section->tracks()[context_.index].bars();
		for(auto&& bar : bars) {

		}
	}

	return true;
}

}
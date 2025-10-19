#include "track.hpp"

#include "imgui.h"

namespace securepath::drum {

bool track::draw()
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

	return true;
}

}
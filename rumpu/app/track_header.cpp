
#include "track_list.hpp"

#include "imgui.h"

namespace securepath::drum::app {

/*track_header::track_header(std::string name)
: child_window_base(std::move(name))
{
}
*/
bool track_header::draw()
{
    auto drawlist = ImGui::GetWindowDrawList();

    auto pos  = ImGui::GetCursorScreenPos();
    auto size = ImGui::GetWindowSize();

    std::size_t i = 0;
    auto tick_pos = pos;
    while(tick_pos.x < pos.x + size.x) {
        auto end = tick_pos;
        if(i++ % 10 == 0) {
            end.y += 10;
        } else {
            end.y += 5;
        }
        drawlist->AddLine(tick_pos, end, IM_COL32(255,127,127,255), 1.0f);
        tick_pos.x += 8;
    }

    return true;
}

}

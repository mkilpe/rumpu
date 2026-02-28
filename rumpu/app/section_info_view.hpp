#pragma once

#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class section_info_view {
public:
    void set_context(song const*, uint32_t section);
    void do_draw();

private:
    song const* song_{};
    uint32_t current_section_{};
};

}

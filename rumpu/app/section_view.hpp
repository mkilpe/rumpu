#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class section_view : public child_window_base {
public:
    section_view(event_system::event_handler& h);

    void set_context(song*, uint32_t section);
    bool do_draw() override;

private:
    event_system::event_handler& handler_;
    song* song_{};
    uint32_t current_section_{};
};

}

#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app { class undo_manager; }

namespace securepath::drum::app {

class section_view : public child_window_base {
public:
    section_view(event_system::event_handler& h);

    void set_context(song*, uint32_t section, undo_manager* undo = nullptr);
    bool do_draw() override;

private:
    undo_manager* undo_{};
    event_system::event_handler& handler_;
    song* song_{};
    uint32_t current_section_{};
};

}

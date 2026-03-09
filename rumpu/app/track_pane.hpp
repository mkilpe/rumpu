#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class track_pane : public child_window_base {
public:
    track_pane(std::string name);

    bool do_draw() override;
    void set_context(event_system::event_handler&, song*, drum::track const&);

private:
    float gain_{};
    event_system::event_handler* handler_{};
    song* song_{};
    std::size_t instrument_index_{};
    std::string display_name_;
};

}
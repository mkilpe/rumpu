#pragma once

#include "child_window.hpp"

#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class toolbar : public child_window_base {
public:
    toolbar(event_system::event_handler& h);
    bool do_draw() override;

    void set_context(song*, uint32_t section);

private:
    void button(const std::string& label);

private:
    event_system::event_handler& handler_;

    song* song_{};
    uint32_t section_{};
};

}
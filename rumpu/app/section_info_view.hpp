#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

class section_info_view : public child_window_base {
public:
    section_info_view();

    void set_context(song const*, uint32_t section);
    bool do_draw() override;

private:
    song const* song_{};
    uint32_t current_section_{};
};

}

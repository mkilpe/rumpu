#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app { class undo_manager; }

#include <string>

namespace securepath::drum::app {

class section_info_view : public child_window_base {
public:
    section_info_view();

    void set_context(song*, uint32_t section, undo_manager* undo = nullptr);
    bool do_draw() override;

private:
    undo_manager* undo_{};
    song* song_{};
    uint32_t current_section_{};
    std::string name_buf_;
};

}

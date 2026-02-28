#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

class track_pane : public child_window_base {
public:
    track_pane(std::string name);

    bool do_draw() override;
    void set_context(song*, std::size_t instrument_index);

private:
    float gain_{};
    song* song_{};
    std::size_t instrument_index_{};
};

}
#pragma once

#include "view.hpp"

namespace securepath::drum::app {

class track_header : public child_window_base {
public:
    track_header();
    bool do_draw() override;

    void set_context(song*, uint32_t section);

private:
    song* song_{};
    uint32_t section_{};
};

}
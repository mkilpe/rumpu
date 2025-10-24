#pragma once

#include "child_window.hpp"

namespace securepath::drum::app {

class track_pane : public child_window_base {
public:
    track_pane(std::string name);

    bool do_draw() override;

private:
    float gain_{};
};

}
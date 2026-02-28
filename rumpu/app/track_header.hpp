#pragma once

#include "view.hpp"

namespace securepath::drum::app {

class track_header : public child_window_base {
public:
    track_header();
    bool do_draw() override;

    void set_context(song*, uint32_t section);
    void set_size(const ImVec2&) override;
    void zoom(float);

private:
    song* song_{};
    uint32_t section_{};
    ImVec2 original_size_{};
};

}
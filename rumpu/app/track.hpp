#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

class track_view : public child_window_base {
public:
    using child_window_base::child_window_base;
    virtual void set_context(size_t index, song*, uint32_t section) = 0;
};

struct track_draw_context;

class track : public track_view {
public:
    track(std::string name);

    bool do_draw() override;
    void set_context(size_t index, song*, uint32_t section) override;
private:
    void toggle_mark(track_draw_context& context, const ImVec2& rel_pos);
    void handle_mouse(track_draw_context&);

private:
    size_t index_{};
    song* song_{};
    uint32_t section_{};
};

using track_ptr = std::unique_ptr<track_view>;

}
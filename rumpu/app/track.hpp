#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app { class undo_manager; }

namespace securepath::drum::app {

class track_view : public child_window_base {
public:
    using child_window_base::child_window_base;
    virtual void set_context(size_t index, song*, uint32_t section, undo_manager* undo = nullptr) = 0;
    virtual void zoom(float) = 0;
};

struct track_draw_context;

class track : public track_view {
public:
    track(std::string name);

    bool do_draw() override;
    void set_context(size_t index, song*, uint32_t section, undo_manager* undo = nullptr) override;
    void set_size(const ImVec2& size) override;
    void zoom(float) override;
private:
    void toggle_mark(track_draw_context&, const ImVec2& rel_pos);
    void handle_mouse(track_draw_context&);
    void context_menu(track_draw_context&);
    void divide_dialog(track_draw_context&);
    void beat_properties_dialog(track_draw_context&);

private:
    undo_manager* undo_{};
    size_t index_{};
    song* song_{};
    uint32_t section_{};
    ImVec2 mouse_pos_{};
    float zoom_{1.0f};
    ImVec2 original_size_{};

    enum class divide_target { beat, bar };
    bool divide_dialog_open_{false};
    divide_target divide_dialog_target_{};
    int divide_amount_{2};
    ImVec2 divide_mouse_pos_{};

    bool beat_props_open_{false};
    ImVec2 beat_props_mouse_pos_{};
    beat* beat_props_beat_{};
};

using track_ptr = std::unique_ptr<track_view>;

}
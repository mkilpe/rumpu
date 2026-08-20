#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>

#include <deque>
#include <string>

namespace securepath::drum { class undo_manager; }

namespace securepath::drum::app {

class track_view : public child_window_base {
public:
    using child_window_base::child_window_base;
    virtual void set_context(size_t index, song*, uint32_t section, undo_manager* undo = nullptr) = 0;
    virtual void zoom(float) = 0;
    // playback state, for disabling edits that don't mix with a running player
    virtual void set_playing(bool) {}
};

struct track_draw_context;

class track : public track_view {
public:
    track(std::string name);

    bool do_draw() override;
    void set_context(size_t index, song*, uint32_t section, undo_manager* undo = nullptr) override;
    void set_size(const ImVec2& size) override;
    void zoom(float) override;
    void set_playing(bool p) override { playing_ = p; }
private:
    enum class divide_target { beat, bar };

    void handle_mouse(track_draw_context&);
    void context_menu(track_draw_context&);
    void beat_menu_items(track_draw_context&, ImVec2 rel);
    void divide_submenu(track_draw_context&, ImVec2 rel, char const* label, divide_target);
    void track_menu_items(track_draw_context&, ImVec2 rel);
    void choke_menu_items(track_draw_context&, ImVec2 rel);
    void seed_apply_pattern(track_draw_context&, ImVec2 rel);
    void divide_dialog(track_draw_context&);
    void beat_properties_dialog(track_draw_context&);
    void hit_beat_properties(beat&);
    void choke_beat_properties(beat&);
    void apply_pattern_dialog(track_draw_context&);
    void apply_pattern_bars_slider(track_draw_context const&);
    void apply_pattern_canvas();
    void apply_pattern_canvas_input(track_draw_context&, bool hovered);
    void apply_pattern_footer(track_draw_context&);
    void apply_pattern_to_track(track_draw_context&);

private:
    undo_manager* undo_{};
    size_t index_{};
    song* song_{};
    uint32_t section_{};
    ImVec2 mouse_pos_{};
    float zoom_{1.0f};
    ImVec2 original_size_{};
    bool playing_{};
    bool divide_dialog_open_{false};
    divide_target divide_dialog_target_{};
    int divide_amount_{2};
    ImVec2 divide_mouse_pos_{};

    bool beat_props_open_{false};
    ImVec2 beat_props_mouse_pos_{};
    beat* beat_props_beat_{};

    bool apply_pattern_open_{false};
    std::deque<bar> apply_pattern_bars_;
    ImVec2 apply_pattern_mouse_pos_{};
    float apply_pattern_zoom_{1.0f};
};

using track_ptr = std::unique_ptr<track_view>;

}
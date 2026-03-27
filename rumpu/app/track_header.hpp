#pragma once

#include "view.hpp"

namespace securepath::drum::app { class undo_manager; }

namespace securepath::drum::app {

class track_header : public child_window_base {
public:
    track_header();
    bool do_draw() override;

    void set_context(song*, uint32_t section, undo_manager* undo = nullptr);
    void set_size(const ImVec2&) override;
    void zoom(float);

private:
    void context_menu(section* sec, ImVec2 header_pos, float lead_x, float bar_width);
    void tempo_dialog(section* sec);

    undo_manager* undo_{};
    song* song_{};
    uint32_t section_{};
    ImVec2 original_size_{};

    ImVec2 mouse_pos_{};
    bool tempo_dialog_open_{false};
    float tempo_dialog_value_{120.0f};
    uint32_t tempo_dialog_bar_{};
};

}
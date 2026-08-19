#pragma once

#include "child_window.hpp"
#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum { class undo_manager; }

namespace securepath::drum::app {

class track_pane : public child_window_base {
public:
    track_pane(std::string name);

    bool do_draw() override;
    void set_context(event_system::event_handler&, song*, std::uint32_t section, std::size_t track_index, drum::track const&, undo_manager* undo = nullptr);

private:
    void clipped_name();
    void mute_button(track_settings&);
    void gain_knob(track_settings*);

private:
    undo_manager* undo_{};
    float gain_{};
    event_system::event_handler* handler_{};
    song* song_{};
    std::uint32_t section_{};
    std::size_t track_index_{};
    std::size_t instrument_index_{};
    std::string display_name_;
};

}
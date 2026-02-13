#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/player.hpp>

#include <securepath/event_system/event_handler.hpp>
#include <securepath/event_system/event_loop.hpp>

#include "child_window.hpp"
#include "track_edit_view.hpp"

namespace securepath::drum::app {

 class rumpu : public event_system::single_thread_event_loop, public event_system::event_handler {
public:
    rumpu();

    bool update();
    void handle_event(std::unique_ptr<event_system::event_base> ev) override;

private:
    void menu();
    void add_track(uint32_t section);
    void play_song(uint32_t section);
    void stop_song(uint32_t section);
    void player_pos_changed();

    void show_about() const;
private:
    bool running_{true};
    bool show_window{true};
    mutable bool show_about_{};

    mutable std::mutex mutex_;
    song song_;
    std::unique_ptr<track_edit_view> track_edit_view_;
    player player_;
    
    std::vector<child_window*> windows_;
};

}

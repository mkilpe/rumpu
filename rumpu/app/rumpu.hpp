#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/player.hpp>

#include <securepath/event_system/event_handler.hpp>
#include <securepath/event_system/event_loop.hpp>

#include "about_dialog.hpp"
#include "add_instrument_dialog.hpp"
#include "export_dialog.hpp"
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
    void select_section(uint32_t section_id);
    void select_section_impl(uint32_t section_id);
    void add_section();
    void add_instrument(std::string path);
    void remove_track(std::size_t index);
    void open_project(std::string path);
    void save_project(std::string path);
    void player_pos_changed();

private:
    bool running_{true};
    std::string current_file_;
    uint32_t current_section_{};
    about_dialog about_dialog_;
    add_instrument_dialog add_instrument_dialog_;
    export_dialog export_dialog_;

    mutable std::mutex mutex_;
    song song_;
    std::unique_ptr<track_edit_view> track_edit_view_;
    player player_;
    
    std::vector<child_window*> windows_;
};

}

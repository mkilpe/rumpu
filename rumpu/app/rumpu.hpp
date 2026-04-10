#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/player.hpp>

#include <securepath/event_system/event_handler.hpp>
#include <securepath/event_system/event_loop.hpp>

#include "app_options.hpp"
#include "about_dialog.hpp"
#include "add_instrument_dialog.hpp"
#include "add_instruments_from_folder_dialog.hpp"
#include "add_track_dialog.hpp"
#include "export_dialog.hpp"
#include "instruments_dialog.hpp"
#include "new_song_dialog.hpp"
#include "song_properties_dialog.hpp"
#include "child_window.hpp"
#include "track_edit_view.hpp"
#include <rumpu/core/undo_manager.hpp>

namespace securepath::drum::app {

 class rumpu : public event_system::single_thread_event_loop, public event_system::event_handler {
public:
    rumpu(app_options options);

    bool update();
    void handle_event(std::unique_ptr<event_system::event_base> ev) override;

private:
    void menu();
    void add_track(uint32_t section, std::size_t instrument_index);
    void open_add_track_dialog(uint32_t section);
    void load_and_play(uint32_t section);
    void play_section(uint32_t section);
    void play_song();
    void stop_song(uint32_t section);
    void select_section(uint32_t section_id);
    void select_section_impl(uint32_t section_id);
    void add_section();
    void add_instrument(std::string path, std::string name);
    void add_instruments(std::vector<std::string> paths);
    void remove_track(std::size_t index);
    void open_project(std::string path);
    void save_project(std::string path);
    void new_song(std::string name, time_signature ts, float tempo);
    void update_song_properties(std::string name, std::string author, std::string notes, time_signature ts, float tempo, float rand_offset_ms, float rand_volume_percent);
    void player_pos_changed();
    void show_error(std::string message);
    void draw_error_dialog();

private:
    bool running_{true};
    std::string current_file_;
    uint32_t current_section_{};
    about_dialog about_dialog_;
    add_instrument_dialog add_instrument_dialog_;
    add_instruments_from_folder_dialog add_instruments_from_folder_dialog_;
    add_track_dialog add_track_dialog_;
    new_song_dialog new_song_dialog_;
    song_properties_dialog song_properties_dialog_;
    instruments_dialog instruments_dialog_;
    export_dialog export_dialog_;

    void perform_undo();
    void perform_redo();

    mutable std::mutex mutex_;
    song song_;
    undo_manager undo_;
    std::unique_ptr<track_edit_view> track_edit_view_;
    player player_;
    
    std::string error_message_;
    bool error_pending_{};
    bool follow_cursor_{true};
    std::vector<child_window*> windows_;
};

}

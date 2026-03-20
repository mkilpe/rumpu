#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/player.hpp>
#include <securepath/event_system/event_handler.hpp>

#include "track.hpp"
#include "track_header.hpp"
#include "track_pane.hpp"
#include "child_window.hpp"

#include <vector>

namespace securepath::drum::app {

class track_list : public child_window_base {
public:
    track_list(std::string name, event_system::event_handler&);
    bool do_draw() override;

    void add_track();
    void set_context(song*, uint32_t section);
    void set_play_status(play_status const&);
private:
    void update_tracks(song* s, uint32_t section);
    void set_track(drum::track const& t);
    void draw_play_cursor(float col_x, float col_top, float col_bottom);
private:
    event_system::event_handler& handler_;
    struct track_info {
        std::unique_ptr<track_pane> pane;
        track_ptr track;
    };
    track_header header_;
    std::vector<track_info> tracks_;

    float zoom_{1.0f};
    song* song_{};
    uint32_t section_{};
    play_status play_status_;
};

}
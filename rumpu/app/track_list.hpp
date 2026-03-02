#pragma once

#include <rumpu/core/song.hpp>
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
private:
    void update_tracks(song* s, uint32_t section);
    void set_track(drum::track const& t);
private:
    event_system::event_handler& handler_;
    struct track_info {
        std::unique_ptr<track_pane> pane;
        track_ptr track;
    };
    track_header header_; 
    std::vector<track_info> tracks_;

    float zoom_{1.0f};
};

}
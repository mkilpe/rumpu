#pragma once

#include <rumpu/core/song.hpp>

#include "track.hpp"
#include "track_header.hpp"
#include "child_window.hpp"

#include <vector>

namespace securepath::drum::app {

class track_list : public child_window_base {
public:
    track_list(std::string name);
    bool do_draw() override;

    void add_track();
    void set_section(song*, uint32_t section);

private:
    struct track_info {
        child_window_ptr pane;
        track_ptr track;
    };
    track_header header_; 
    std::vector<track_info> tracks_;
};

}
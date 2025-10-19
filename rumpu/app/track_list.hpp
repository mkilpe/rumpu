#pragma once

#include "track.hpp"
#include "track_header.hpp"
#include "child_window.hpp"

#include <vector>

namespace securepath::drum {

class track_list : public view {
public:
    track_list();    
    bool draw() override;

    void add_track();

private:
    struct track_info {
        child_window_ptr pane;
        child_window_ptr track;
    };
    track_header header_; 
    std::vector<track_info> tracks_;
};

}
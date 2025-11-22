#pragma once

#include "view.hpp"
#include "toolbar.hpp"
#include "track_list.hpp"

namespace securepath::drum::app {

class track_edit_view : public child_window_base{
public:
    track_edit_view(event_system::event_handler&);
    bool do_draw() override;

    void set_context(song*, uint32_t section);

private:
    toolbar toolbar_;
    track_list track_list_;
};

}
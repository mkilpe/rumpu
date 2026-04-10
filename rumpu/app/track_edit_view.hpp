#pragma once

#include "view.hpp"
#include "toolbar.hpp"
#include "section_view.hpp"
#include "section_info_view.hpp"
#include "track_list.hpp"

namespace securepath::drum::app {

class track_edit_view : public child_window_base{
public:
    track_edit_view(event_system::event_handler&);
    bool do_draw() override;

    void set_context(song*, uint32_t section, undo_manager* undo = nullptr);
    void set_play_status(play_status const&);
    void set_follow_cursor(bool v) { track_list_.set_follow_cursor(v); }
    bool follow_cursor() const { return track_list_.follow_cursor(); }

private:
    toolbar toolbar_;
    section_view section_view_;
    section_info_view section_info_view_;
    track_list track_list_;
};

}
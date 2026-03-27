#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

class song_properties_dialog : public view {
public:
    explicit song_properties_dialog(event_system::event_handler&);
    void open(song const*);
    bool draw() override;
private:
    ui_task run(song const* s);

    event_system::event_handler& handler_;
    ui_task task_;
};

}

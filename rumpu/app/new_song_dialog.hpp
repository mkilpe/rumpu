#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class new_song_dialog : public view {
public:
    explicit new_song_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    ui_task run();

    event_system::event_handler& handler_;
    ui_task task_;
};

}

#pragma once
#include "view.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <string>

namespace securepath::drum::app {

class new_song_dialog : public view {
public:
    explicit new_song_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    event_system::event_handler& handler_;
    bool open_{};
    char name_[256]{};
    int beats_{4};
    int beat_type_{4};
    float tempo_{120.0f};
};

}

#pragma once
#include "view.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <rumpu/core/song.hpp>
#include <string>

namespace securepath::drum::app {

class song_properties_dialog : public view {
public:
    explicit song_properties_dialog(event_system::event_handler&);
    void open(song const*);
    bool draw() override;
private:
    event_system::event_handler& handler_;
    bool open_{};
    char name_[256]{};
    char author_[256]{};
    char notes_[1024]{};
    int beats_{4};
    int beat_type_{4};
    float tempo_{120.0f};
};

}

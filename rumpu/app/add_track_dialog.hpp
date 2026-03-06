#pragma once
#include "view.hpp"
#include <rumpu/core/song.hpp>
#include <securepath/event_system/event_handler.hpp>
#include <cstdint>

namespace securepath::drum::app {

class add_track_dialog : public view {
public:
    explicit add_track_dialog(event_system::event_handler&);
    void open(song* s, std::uint32_t section);
    bool draw() override;
private:
    void draw_content();

    event_system::event_handler& handler_;
    song* song_{};
    std::uint32_t section_{};
    bool open_{};
    int selected_{-1};
};

}

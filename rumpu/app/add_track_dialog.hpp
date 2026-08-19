#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
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
    ui_task run(song* s, std::uint32_t section);
    void instrument_list(song const& s, int& selected);

    event_system::event_handler& handler_;
    ui_task task_;
};

}

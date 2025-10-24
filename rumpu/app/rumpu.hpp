#pragma once

#include <rumpu/core/song.hpp>

#include <securepath/event_system/event_handler.hpp>
#include <securepath/event_system/event_loop.hpp>

#include "child_window.hpp"

namespace securepath::drum::app {

 class rumpu : public event_system::single_thread_event_loop, public event_system::event_handler {
public:
    rumpu();

    bool update();
    void handle_event(std::unique_ptr<event_system::event_base> ev) override;

private:
    void menu();
private:
    bool running_{true};
    bool show_window{true};

    mutable std::mutex mutex_;
    std::vector<child_window_ptr> windows_;
    song song_;
};

}

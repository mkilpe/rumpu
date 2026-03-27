#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <mutex>
#include <string>

namespace securepath::drum::app {

class add_instrument_dialog : public view {
public:
    explicit add_instrument_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    ui_task run();
    std::string collect_file_result();

    event_system::event_handler& handler_;
    ui_task task_;

    // Shared with file dialog callback thread
    std::mutex result_mutex_;
    bool has_result_{};
    std::string result_path_;
};

}

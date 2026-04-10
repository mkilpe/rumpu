#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <mutex>
#include <string>

namespace securepath::drum::app {

class add_instruments_from_folder_dialog : public view {
public:
    explicit add_instruments_from_folder_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    ui_task run();
    std::string collect_folder_result();

    event_system::event_handler& handler_;
    ui_task task_;

    std::mutex result_mutex_;
    bool has_result_{};
    std::string result_folder_;
};

}

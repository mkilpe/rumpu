#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <securepath/event_system/event_handler.hpp>

namespace securepath::drum::app {

class add_instruments_from_folder_dialog : public view {
public:
    explicit add_instruments_from_folder_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    ui_task run();

    event_system::event_handler& handler_;
    ui_task task_;
    // Shared with folder dialog callback thread; must outlive this object
    async_dialog_result_ptr folder_result_ = std::make_shared<async_dialog_result>();
};

}

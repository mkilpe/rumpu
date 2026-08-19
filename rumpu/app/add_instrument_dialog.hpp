#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <securepath/event_system/event_handler.hpp>
#include <string>

namespace securepath::drum::app {

class add_instrument_dialog : public view {
public:
    explicit add_instrument_dialog(event_system::event_handler&);
    void open();
    bool draw() override;
private:
    ui_task run();
    void path_row(std::string& path);

    event_system::event_handler& handler_;
    ui_task task_;
    // Shared with file dialog callback thread; must outlive this object
    async_dialog_result_ptr file_result_ = std::make_shared<async_dialog_result>();
};

}

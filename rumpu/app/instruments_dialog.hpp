#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <rumpu/core/song.hpp>
#include <rumpu/core/undo_manager.hpp>

namespace securepath::drum::app {

class instruments_dialog : public view {
public:
    instruments_dialog() = default;
    void open(song* s, undo_manager* undo);
    bool draw() override;

private:
    ui_task run(song* s, undo_manager* undo);

    ui_task task_;
    // Shared with file dialog callback thread; must outlive this object
    async_dialog_result_ptr file_result_ = std::make_shared<async_dialog_result>();
};

}

#pragma once

#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <rumpu/core/song.hpp>

namespace securepath::drum::app {

class export_dialog : public view {
public:
    // Takes ownership of a song snapshot; instruments must already be loaded
    // at the export sample rate.
    void open(song);
    bool draw() override;

private:
    ui_task run(song s);

    ui_task task_;
    // Shared with file dialog callback thread; must outlive this object
    async_dialog_result_ptr file_result_ = std::make_shared<async_dialog_result>();
};

}

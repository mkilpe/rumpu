#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include <rumpu/core/song.hpp>
#include <rumpu/core/undo_manager.hpp>

#include <mutex>
#include <string>

namespace securepath::drum::app {

class instruments_dialog : public view {
public:
    instruments_dialog() = default;
    void open(song* s, undo_manager* undo);
    bool draw() override;

private:
    ui_task run(song* s, undo_manager* undo);
    std::string collect_file_result();

    ui_task task_;

    std::mutex result_mutex_;
    bool has_result_{};
    std::string result_path_;
};

}

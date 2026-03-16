#pragma once

#include "view.hpp"
#include "ui_coroutine.hpp"
#include <rumpu/core/song.hpp>
#include <mutex>
#include <string>

namespace securepath::drum::app {

class export_dialog : public view {
public:
    void open(song const*);
    bool draw() override;

private:
    ui_task run(song const* s);
    std::string collect_file_result();

    ui_task task_;

    // Shared with file dialog callback thread
    std::mutex file_mutex_;
    bool has_file_{};
    std::string file_result_;
};

}

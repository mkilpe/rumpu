#pragma once
#include <securepath/event_system/event_handler.hpp>
#include <mutex>
#include <string>

namespace securepath::drum::app {

class add_instrument_dialog {
public:
    explicit add_instrument_dialog(event_system::event_handler&);
    void open();
    void do_draw();
private:
    void on_file_selected(std::string path);

    event_system::event_handler& handler_;
    bool open_{};
    bool browsing_{};
    char path_buf_[512]{};

    std::mutex result_mutex_;
    bool has_result_{};
    std::string result_path_;
};

}

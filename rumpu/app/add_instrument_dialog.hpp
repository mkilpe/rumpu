#pragma once
#include "view.hpp"
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
    void on_file_selected(std::string path);
    void collect_result();
    void draw_content();

    event_system::event_handler& handler_;
    bool open_{};
    bool browsing_{};
    std::string path_;

    std::mutex result_mutex_;
    bool has_result_{};
    std::string result_path_;
};

}

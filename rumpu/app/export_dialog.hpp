#pragma once

#include "view.hpp"
#include <rumpu/core/export.hpp>
#include <rumpu/core/song.hpp>
#include <mutex>
#include <optional>
#include <string>

namespace securepath::drum::app {

class export_dialog : public view {
public:
    void open(song const*);
    bool draw() override;
private:
    void on_file_selected(std::string path);
    void collect_file_result();
    void tick_export();
    void draw_content();
    void draw_idle_content();
    void start_export();

    song const* song_{};
    bool open_{};
    bool browsing_{};
    std::string path_;
    export_options options_;

    std::mutex file_mutex_;
    bool has_file_{};
    std::string file_result_;

    enum class state { idle, exporting, done, failed };
    state state_{state::idle};
    std::optional<wav_exporter> exporter_;
    std::string export_error_;
};

}

#pragma once

#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <rumpu/core/export.hpp>
#include <rumpu/core/song.hpp>

#include <filesystem>
#include <optional>

namespace securepath::drum::app {

class export_dialog : public view {
public:
    // Takes ownership of a song snapshot; its samples are loaded at the
    // export sample rate, with progress shown, once the user starts the
    // export (relative sample paths resolve against project_base).
    void open(song, std::filesystem::path project_base);
    bool draw() override;

private:
    enum class export_action { none, start, cancel };

    ui_task run(song s, std::filesystem::path project_base);
    export_action options_frame(std::string& path, export_options& options);
    void create_exporter(std::string const& path, song, export_options const&,
        std::filesystem::path project_base, std::optional<wav_exporter>&, std::string& error);
    void export_step(wav_exporter&, std::string& error, bool& done);
    bool completed_frame(wav_exporter const&); // true when Close was pressed
    bool failed_frame(std::string const& error);

    ui_task task_;
    // Shared with file dialog callback thread; must outlive this object
    async_dialog_result_ptr file_result_ = std::make_shared<async_dialog_result>();
};

}

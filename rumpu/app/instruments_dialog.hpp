#pragma once
#include "view.hpp"
#include "ui_coroutine.hpp"
#include "async_dialog_result.hpp"
#include <rumpu/core/song.hpp>
#include <rumpu/core/undo_manager.hpp>

#include <filesystem>

namespace securepath::drum::app {

class instruments_dialog : public view {
public:
    instruments_dialog() = default;
    void open(song* s, undo_manager* undo, std::filesystem::path project_base);
    bool draw() override;

private:
    ui_task run(song* s, undo_manager* undo, std::filesystem::path project_base);
    void handle_added_sample(song& s, undo_manager* undo, std::filesystem::path const& project_base, int selected);
    void panels(song& s, undo_manager* undo, int& selected, int& selected_sample);
    void instrument_list(song& s, int& selected, int& selected_sample);
    void instrument_properties(song& s, undo_manager* undo, instrument& inst);
    void sample_list(instrument const& inst, int& selected_sample);
    void sample_buttons(song& s, undo_manager* undo, instrument& inst, int& selected_sample);

    ui_task task_;
    // Shared with file dialog callback thread; must outlive this object
    async_dialog_result_ptr file_result_ = std::make_shared<async_dialog_result>();
};

}

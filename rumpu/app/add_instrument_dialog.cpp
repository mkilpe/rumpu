
#include "add_instrument_dialog.hpp"
#include "events.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"

#include <cstring>

namespace securepath::drum::app {

add_instrument_dialog::add_instrument_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_instrument_dialog::open() {
    open_ = true;
    path_buf_[0] = '\0';
}

void add_instrument_dialog::on_file_selected(std::string path) {
    std::lock_guard l{result_mutex_};
    result_path_ = std::move(path);
    has_result_ = true;
}

void add_instrument_dialog::do_draw() {
    {
        std::lock_guard l{result_mutex_};
        if (has_result_) {
            std::strncpy(path_buf_, result_path_.c_str(), sizeof(path_buf_) - 1);
            path_buf_[sizeof(path_buf_) - 1] = '\0';
            has_result_ = false;
            browsing_ = false;
        }
    }

    if (open_) {
        ImGui::OpenPopup("Add instrument");
        open_ = false;
    }

    if (ImGui::BeginPopupModal("Add instrument", nullptr, ImGuiWindowFlags_None)) {
        ImGui::Text("WAV file path:");
        ImGui::InputText("##path", path_buf_, sizeof(path_buf_));
        ImGui::SameLine();

        bool const was_browsing = browsing_;
        if (was_browsing) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Browse...")) {
            browsing_ = true;
            open_wav_file_dialog([this](std::string path) {
                on_file_selected(std::move(path));
            });
        }
        if (was_browsing) {
            ImGui::EndDisabled();
        }

        if (ImGui::Button("Add")) {
            handler_.emit<event::add_instrument>(std::string(path_buf_));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

}


#include "add_instrument_dialog.hpp"
#include "events.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

namespace securepath::drum::app {

add_instrument_dialog::add_instrument_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_instrument_dialog::open() {
    open_ = true;
    path_.clear();
}

void add_instrument_dialog::on_file_selected(std::string path) {
    std::lock_guard l{result_mutex_};
    result_path_ = std::move(path);
    has_result_ = true;
}

void add_instrument_dialog::collect_result() {
    std::lock_guard l{result_mutex_};
    if (has_result_) {
        path_ = std::move(result_path_);
        has_result_ = false;
        browsing_ = false;
    }
}

void add_instrument_dialog::draw_content() {
    ImGui::Text("WAV file path:");
    ImGui::InputText("##path", &path_);
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
        handler_.emit<event::add_instrument>(path_);
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        ImGui::CloseCurrentPopup();
    }
}

bool add_instrument_dialog::draw() {
    collect_result();

    if (open_) {
        ImGui::OpenPopup("Add instrument");
        open_ = false;
    }

    if (ImGui::BeginPopupModal("Add instrument", nullptr, ImGuiWindowFlags_None)) {
        draw_content();
        ImGui::EndPopup();
    }
    return true;
}

}

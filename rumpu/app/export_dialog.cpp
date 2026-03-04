
#include "export_dialog.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

namespace securepath::drum::app {

void export_dialog::open(song const* s) {
    song_ = s;
    state_ = state::idle;
    exporter_.reset();
    path_.clear();
    open_ = true;
}

void export_dialog::on_file_selected(std::string path) {
    std::lock_guard l{file_mutex_};
    file_result_ = std::move(path);
    has_file_ = true;
}

void export_dialog::start_export() {
    state_ = state::exporting;
    exporter_.emplace(path_, *song_, options_);
}

void export_dialog::do_draw() {
    {
        std::lock_guard l{file_mutex_};
        if (has_file_) {
            path_ = std::move(file_result_);
            has_file_ = false;
            browsing_ = false;
        }
    }

    if (state_ == state::exporting) {
        try {
            if (!exporter_->process()) {
                state_ = state::done;
            }
        } catch (std::exception const& e) {
            export_error_ = e.what();
            state_ = state::failed;
            exporter_.reset();
        }
    }

    if (open_) {
        ImGui::OpenPopup("Export as WAV");
        open_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({420, 180}, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Export as WAV", nullptr, ImGuiWindowFlags_None)) {
        if (state_ == state::exporting) {
            ImGui::Text("Exporting... %.3fs", exporter_->progress());
        } else if (state_ == state::done) {
            ImGui::Text("Exporting... %.3fs", exporter_->progress());
            ImGui::Text("Export complete.");
            if (ImGui::Button("Close")) {
                state_ = state::idle;
                ImGui::CloseCurrentPopup();
            }
        } else if (state_ == state::failed) {
            ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "Export failed: %s", export_error_.c_str());
            if (ImGui::Button("Close")) {
                state_ = state::idle;
                ImGui::CloseCurrentPopup();
            }
        } else {
            ImGui::Text("Output file:");
            ImGui::SetNextItemWidth(-90);
            ImGui::InputText("##path", &path_);
            ImGui::SameLine();

            bool const was_browsing = browsing_;
            if (was_browsing) { ImGui::BeginDisabled(); }
            if (ImGui::Button("Browse...")) {
                browsing_ = true;
                save_wav_file_dialog([this](std::string path) {
                    on_file_selected(std::move(path));
                });
            }
            if (was_browsing) { ImGui::EndDisabled(); }

            ImGui::SeparatorText("Gain control");
            int gain = static_cast<int>(options_.gain_control);
            ImGui::RadioButton("Peak normalise", &gain, export_options::peak_normalise);
            ImGui::RadioButton("None", &gain, export_options::none);
            options_.gain_control = static_cast<export_options::gain_control_type>(gain);

            ImGui::Separator();
            bool const path_empty = path_.empty();
            if (path_empty) { ImGui::BeginDisabled(); }
            if (ImGui::Button("Export")) {
                start_export();
            }
            if (path_empty) { ImGui::EndDisabled(); }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndPopup();
    }
}

}

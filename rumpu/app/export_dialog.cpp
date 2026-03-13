
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

void export_dialog::collect_file_result() {
    std::lock_guard l{file_mutex_};
    if (has_file_) {
        path_ = std::move(file_result_);
        has_file_ = false;
        browsing_ = false;
    }
}

void export_dialog::tick_export() {
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

void export_dialog::draw_idle_content() {
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
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
    }
}

void export_dialog::draw_content() {
    auto draw_progress = [this] {
        float current = exporter_->progress();
        float total = exporter_->duration();
        float fraction = total > 0 ? current / total : 0.0f;
        if (fraction > 1.0f) fraction = 1.0f;

        auto cur_ms = static_cast<long long>(current * 1000);
        auto tot_ms = static_cast<long long>(total * 1000);
        char overlay[64];
        std::snprintf(overlay, sizeof(overlay), "%lld:%02lld / %lld:%02lld",
            cur_ms / 60000, (cur_ms / 1000) % 60,
            tot_ms / 60000, (tot_ms / 1000) % 60);

        ImGui::ProgressBar(fraction, {-1, 0}, overlay);
    };
    if (state_ == state::exporting) {
        ImGui::Text("Exporting...");
        draw_progress();
    } else if (state_ == state::done) {
        ImGui::Text("Export complete.");
        draw_progress();
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
        draw_idle_content();
    }
}

bool export_dialog::draw() {
    collect_file_result();

    if (state_ == state::exporting) {
        tick_export();
    }

    if (open_) {
        ImGui::OpenPopup("Export as WAV");
        open_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({420, 180}, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("Export as WAV", nullptr, ImGuiWindowFlags_None)) {
        draw_content();
        ImGui::EndPopup();
    }
    return true;
}

}


#include "export_dialog.hpp"
#include "native_file_dialog.hpp"
#include <rumpu/core/export.hpp>

#include "imgui.h"
#include "imgui_stdlib.h"

#include <optional>

namespace securepath::drum::app {

static bool begin_export_popup() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({420, 220}, ImGuiCond_FirstUseEver);
    return ImGui::BeginPopupModal("Export as WAV", nullptr, ImGuiWindowFlags_None);
}

static void draw_progress(wav_exporter const& exporter) {
    float current = exporter.progress();
    float total = exporter.duration();
    float fraction = total > 0 ? current / total : 0.0f;
    if (fraction > 1.0f) {
        fraction = 1.0f;
    }

    auto cur_ms = static_cast<long long>(current * 1000);
    auto tot_ms = static_cast<long long>(total * 1000);
    char overlay[64];
    std::snprintf(overlay, sizeof(overlay), "%lld:%02lld / %lld:%02lld",
        cur_ms / 60000, (cur_ms / 1000) % 60,
        tot_ms / 60000, (tot_ms / 1000) % 60);

    ImGui::ProgressBar(fraction, {-1, 0}, overlay);
}

void export_dialog::open(song s) {
    // Drop any result or still-open chooser from a previous dialog session
    file_result_->invalidate();
    task_ = run(std::move(s));
}

ui_task export_dialog::run(song s) {
    ImGui::OpenPopup("Export as WAV");
    co_await next_frame{};

    std::string path;
    export_options options;

    // Phase 1: show options until the user clicks Export or Cancel
    export_action action = export_action::none;
    while (action == export_action::none) {
        if (!begin_export_popup()) {
            co_return;
        }
        action = options_frame(path, options);
        ImGui::EndPopup();
        if (action == export_action::cancel) {
            co_return;
        }
        co_await next_frame{};
    }

    // Phase 2: one exporter chunk per frame, then a done/failed screen
    std::string error;
    std::optional<wav_exporter> exporter;
    try {
        exporter.emplace(path, std::move(s), options);
    } catch (std::exception const& e) {
        error = e.what();
    }

    bool done = false;
    while (true) {
        if (!begin_export_popup()) {
            co_return;
        }
        bool close = false;
        if (!error.empty()) {
            close = failed_frame(error);
        } else if (!done) {
            export_step(*exporter, error, done);
        } else {
            close = completed_frame(*exporter);
        }
        ImGui::EndPopup();
        if (close) {
            co_return;
        }
        co_await next_frame{};
    }
}

export_dialog::export_action export_dialog::options_frame(std::string& path, export_options& options) {
    if (auto result = file_result_->take(); result && !result->empty()) {
        path = std::move(*result);
    }

    ImGui::Text("Output file:");
    ImGui::SetNextItemWidth(-90);
    ImGui::InputText("##path", &path);
    ImGui::SameLine();

    bool const browsing = file_result_->in_flight();
    if (browsing) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Browse...")) {
        if (auto session = file_result_->begin()) {
            save_wav_file_dialog([r = file_result_, s = *session](std::string p) {
                r->deliver(s, std::move(p));
            });
        }
    }
    if (browsing) {
        ImGui::EndDisabled();
    }

    ImGui::SeparatorText("Gain control");
    int gain = static_cast<int>(options.gain_control);
    ImGui::RadioButton("Peak normalise", &gain, export_options::peak_normalise);
    ImGui::RadioButton("None", &gain, export_options::none);
    options.gain_control = static_cast<export_options::gain_control_type>(gain);

    ImGui::Separator();
    bool const path_empty = path.empty();
    if (path_empty) {
        ImGui::BeginDisabled();
    }
    export_action action = ImGui::Button("Export") ? export_action::start : export_action::none;
    if (path_empty) {
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
        action = export_action::cancel;
    }
    return action;
}

// run one exporter chunk and draw the progress frame
void export_dialog::export_step(wav_exporter& exporter, std::string& error, bool& done) {
    try {
        done = !exporter.process();
    } catch (std::exception const& e) {
        error = e.what();
    }
    ImGui::Text("Exporting...");
    draw_progress(exporter);
}

bool export_dialog::completed_frame(wav_exporter const& exporter) {
    ImGui::Text("Export complete.");
    draw_progress(exporter);
    bool const close = ImGui::Button("Close");
    if (close) {
        ImGui::CloseCurrentPopup();
    }
    return close;
}

bool export_dialog::failed_frame(std::string const& error) {
    ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "Export failed: %s", error.c_str());
    bool const close = ImGui::Button("Close");
    if (close) {
        ImGui::CloseCurrentPopup();
    }
    return close;
}

bool export_dialog::draw() {
    task_.tick();
    return true;
}

}


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
    ImGui::SetNextWindowSize({420, 180}, ImGuiCond_Appearing);
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

void export_dialog::open(song const* s) {
    task_ = run(s);
}

std::string export_dialog::collect_file_result() {
    std::lock_guard l{file_mutex_};
    if (has_file_) {
        has_file_ = false;
        return std::move(file_result_);
    }
    return {};
}

ui_task export_dialog::run(song const* s) {
    ImGui::OpenPopup("Export as WAV");
    co_await next_frame{};

    std::string path;
    export_options options;
    bool browsing = false;

    // Phase 1: Show options until user clicks Export or Cancel
    while (true) {
        if (auto result = collect_file_result(); !result.empty()) {
            path = std::move(result);
            browsing = false;
        }

        if (!begin_export_popup()) {
            co_return;
        }

        ImGui::Text("Output file:");
        ImGui::SetNextItemWidth(-90);
        ImGui::InputText("##path", &path);
        ImGui::SameLine();

        bool const was_browsing = browsing;
        if (was_browsing) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Browse...")) {
            browsing = true;
            save_wav_file_dialog([this](std::string p) {
                std::lock_guard l{file_mutex_};
                file_result_ = std::move(p);
                has_file_ = true;
            });
        }
        if (was_browsing) {
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
        bool const do_export = ImGui::Button("Export");
        if (path_empty) {
            ImGui::EndDisabled();
        }
        if (do_export) {
            ImGui::EndPopup();
            break;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }

        ImGui::EndPopup();
        co_await next_frame{};
    }

    // Phase 2: Export with progress
    std::string error;
    try {
        wav_exporter exporter(path, *s, options);

        while (exporter.process()) {
            if (!begin_export_popup()) {
                co_return;
            }
            ImGui::Text("Exporting...");
            draw_progress(exporter);
            ImGui::EndPopup();
            co_await next_frame{};
        }

        // Phase 3: Done
        while (true) {
            if (!begin_export_popup()) {
                co_return;
            }
            ImGui::Text("Export complete.");
            draw_progress(exporter);
            if (ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                co_return;
            }
            ImGui::EndPopup();
            co_await next_frame{};
        }
    } catch (std::exception const& e) {
        error = e.what();
    }

    // Phase 3: Failed
    while (true) {
        if (!begin_export_popup()) {
            co_return;
        }
        ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "Export failed: %s", error.c_str());
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }
        ImGui::EndPopup();
        co_await next_frame{};
    }
}

bool export_dialog::draw() {
    task_.tick();
    return true;
}

}

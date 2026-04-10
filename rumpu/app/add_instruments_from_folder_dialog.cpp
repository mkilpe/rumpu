
#include "add_instruments_from_folder_dialog.hpp"
#include "events.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include <algorithm>
#include <filesystem>
#include <system_error>
#include <vector>

namespace securepath::drum::app {

add_instruments_from_folder_dialog::add_instruments_from_folder_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_instruments_from_folder_dialog::open() {
    task_ = run();
}

std::string add_instruments_from_folder_dialog::collect_folder_result() {
    std::lock_guard l{result_mutex_};
    if (has_result_) {
        has_result_ = false;
        return std::move(result_folder_);
    }
    return {};
}

static bool is_wav_extension(std::filesystem::path const& p) {
    auto ext = p.extension().string();
    return ext == ".wav" || ext == ".WAV";
}

static void scan_folder(std::string const& folder, bool recursive,
                        std::vector<std::filesystem::path>& out_paths,
                        std::string& out_error) {
    namespace fs = std::filesystem;
    out_paths.clear();
    out_error.clear();

    if (folder.empty()) {
        return;
    }

    try {
        if (recursive) {
            for (auto const& entry : fs::recursive_directory_iterator{folder}) {
                if (entry.is_regular_file() && is_wav_extension(entry.path())) {
                    out_paths.push_back(entry.path());
                }
            }
        } else {
            for (auto const& entry : fs::directory_iterator{folder}) {
                if (entry.is_regular_file() && is_wav_extension(entry.path())) {
                    out_paths.push_back(entry.path());
                }
            }
        }
        std::sort(out_paths.begin(), out_paths.end());
    } catch (std::exception const& e) {
        out_error = e.what();
        out_paths.clear();
    }
}

ui_task add_instruments_from_folder_dialog::run() {
    ImGui::OpenPopup("Add instruments from folder");
    co_await next_frame{};

    std::string folder;
    bool recursive = false;
    bool browsing = false;

    std::string scanned_folder;
    bool scanned_recursive = false;
    std::vector<std::filesystem::path> scan_paths;
    std::vector<bool> selected;
    std::string scan_error;

    while (true) {
        if (auto result = collect_folder_result(); !result.empty()) {
            folder = std::move(result);
            browsing = false;
        }

        ImGui::SetNextWindowSize({640, 480}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Add instruments from folder", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        ImGui::Text("Folder:");
        float browse_width = ImGui::CalcTextSize("Browse...").x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetNextItemWidth(-browse_width);
        ImGui::InputText("##folder", &folder);
        ImGui::SameLine();

        bool const was_browsing = browsing;
        if (was_browsing) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Browse...")) {
            browsing = true;
            open_folder_dialog([this](std::string p) {
                std::lock_guard l{result_mutex_};
                result_folder_ = std::move(p);
                has_result_ = true;
            });
        }
        if (was_browsing) {
            ImGui::EndDisabled();
        }

        ImGui::Checkbox("Recursive (include subfolders)", &recursive);

        if (folder != scanned_folder || recursive != scanned_recursive) {
            scanned_folder = folder;
            scanned_recursive = recursive;
            scan_folder(folder, recursive, scan_paths, scan_error);
            selected.assign(scan_paths.size(), false);
        }

        std::size_t const selected_count = static_cast<std::size_t>(
            std::count(selected.begin(), selected.end(), true));

        ImGui::Separator();

        if (!scan_error.empty()) {
            ImGui::TextColored({1.0f, 0.5f, 0.5f, 1.0f}, "Error: %s", scan_error.c_str());
        } else if (folder.empty()) {
            ImGui::TextDisabled("Select a folder to preview WAV files.");
        } else if (scan_paths.empty()) {
            ImGui::TextDisabled("No WAV files found.");
        } else if (selected_count > 0) {
            ImGui::Text("%zu of %zu selected (click rows to toggle)",
                        selected_count, scan_paths.size());
        } else {
            ImGui::Text("%zu WAV file(s) — none selected, all will be imported",
                        scan_paths.size());
        }

        bool const have_paths = !scan_paths.empty() && scan_error.empty();
        if (!have_paths) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Select all")) {
            selected.assign(scan_paths.size(), true);
        }
        ImGui::SameLine();
        if (ImGui::Button("Select none")) {
            selected.assign(scan_paths.size(), false);
        }
        ImGui::SameLine();
        if (ImGui::Button("Invert")) {
            for (std::size_t i = 0; i < selected.size(); ++i) {
                selected[i] = !selected[i];
            }
        }
        if (!have_paths) {
            ImGui::EndDisabled();
        }

        float const button_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        if (ImGui::BeginListBox("##files", ImVec2(-FLT_MIN, -button_height))) {
            namespace fs = std::filesystem;
            for (std::size_t i = 0; i < scan_paths.size(); ++i) {
                auto const& p = scan_paths[i];
                std::error_code ec;
                auto rel = fs::relative(p, folder, ec);
                std::string label = (ec || rel.empty()) ? p.string() : rel.string();

                ImGui::PushID(static_cast<int>(i));
                bool is_selected = selected[i];
                if (ImGui::Selectable(label.c_str(), is_selected)) {
                    selected[i] = !is_selected;
                }
                ImGui::PopID();
            }
            ImGui::EndListBox();
        }

        bool const can_import = !scan_paths.empty() && scan_error.empty();
        if (!can_import) {
            ImGui::BeginDisabled();
        }
        bool const import_clicked = ImGui::Button("Import");
        if (!can_import) {
            ImGui::EndDisabled();
        }
        if (import_clicked) {
            std::vector<std::string> paths;
            paths.reserve(scan_paths.size());
            bool const filter = selected_count > 0;
            for (std::size_t i = 0; i < scan_paths.size(); ++i) {
                if (!filter || selected[i]) {
                    paths.push_back(scan_paths[i].string());
                }
            }
            handler_.emit<event::add_instruments>(std::move(paths));
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
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
}

bool add_instruments_from_folder_dialog::draw() {
    task_.tick();
    return true;
}

}

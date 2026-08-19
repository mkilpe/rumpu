
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
    // Drop any result or still-open chooser from a previous dialog session
    folder_result_->invalidate();
    task_ = run();
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

struct add_instruments_from_folder_dialog::scan_state {
    std::string folder;
    bool recursive = false;

    std::string scanned_folder;
    bool scanned_recursive = false;
    std::vector<std::filesystem::path> scan_paths;
    std::vector<bool> selected;
    std::string scan_error;
    // scanning happens after the folder text has been stable for a moment, so
    // typing a path does not hit the filesystem on every keystroke
    double changed_time = -1.0;
    bool scan_now = false;

    std::size_t selected_count() const {
        return static_cast<std::size_t>(std::count(selected.begin(), selected.end(), true));
    }
    bool have_paths() const { return !scan_paths.empty() && scan_error.empty(); }
};

ui_task add_instruments_from_folder_dialog::run() {
    ImGui::OpenPopup("Add instruments from folder");
    co_await next_frame{};

    scan_state state;
    while (true) {
        if (auto result = folder_result_->take(); result && !result->empty()) {
            state.folder = std::move(*result);
            state.scan_now = true;
        }

        ImGui::SetNextWindowSize({640, 480}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Add instruments from folder", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        folder_row(state);
        update_scan(state);
        ImGui::Separator();
        selection_controls(state);
        file_list(state);
        bool const close = import_footer(state);
        ImGui::EndPopup();
        if (close) {
            co_return;
        }
        co_await next_frame{};
    }
}

void add_instruments_from_folder_dialog::folder_row(scan_state& state) {
    ImGui::Text("Folder:");
    float browse_width = ImGui::CalcTextSize("Browse...").x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetNextItemWidth(-browse_width);
    ImGui::InputText("##folder", &state.folder);
    ImGui::SameLine();

    bool const browsing = folder_result_->in_flight();
    if (browsing) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Browse...")) {
        if (auto session = folder_result_->begin()) {
            open_folder_dialog([r = folder_result_, s = *session](std::string p) {
                r->deliver(s, std::move(p));
            });
        }
    }
    if (browsing) {
        ImGui::EndDisabled();
    }
}

void add_instruments_from_folder_dialog::update_scan(scan_state& state) {
    if (ImGui::Checkbox("Recursive (include subfolders)", &state.recursive)) {
        state.scan_now = true;
    }

    bool const dirty = state.folder != state.scanned_folder || state.recursive != state.scanned_recursive;
    if (dirty) {
        if (state.changed_time < 0.0) {
            state.changed_time = ImGui::GetTime();
        }
        if (state.scan_now || ImGui::GetTime() - state.changed_time > 0.4) {
            state.scanned_folder = state.folder;
            state.scanned_recursive = state.recursive;
            scan_folder(state.folder, state.recursive, state.scan_paths, state.scan_error);
            state.selected.assign(state.scan_paths.size(), false);
            state.changed_time = -1.0;
        }
    } else {
        state.changed_time = -1.0;
    }
    state.scan_now = false;
}

void add_instruments_from_folder_dialog::selection_controls(scan_state& state) {
    if (!state.scan_error.empty()) {
        ImGui::TextColored({1.0f, 0.5f, 0.5f, 1.0f}, "Error: %s", state.scan_error.c_str());
    } else if (state.folder.empty()) {
        ImGui::TextDisabled("Select a folder to preview WAV files.");
    } else if (state.scan_paths.empty()) {
        ImGui::TextDisabled("No WAV files found.");
    } else if (state.selected_count() > 0) {
        ImGui::Text("%zu of %zu selected (click rows to toggle)",
                    state.selected_count(), state.scan_paths.size());
    } else {
        ImGui::Text("%zu WAV file(s) — none selected, all will be imported",
                    state.scan_paths.size());
    }

    if (!state.have_paths()) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Select all")) {
        state.selected.assign(state.scan_paths.size(), true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Select none")) {
        state.selected.assign(state.scan_paths.size(), false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Invert")) {
        for (std::size_t i = 0; i < state.selected.size(); ++i) {
            state.selected[i] = !state.selected[i];
        }
    }
    if (!state.have_paths()) {
        ImGui::EndDisabled();
    }
}

void add_instruments_from_folder_dialog::file_list(scan_state& state) {
    float const button_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    if (ImGui::BeginListBox("##files", ImVec2(-FLT_MIN, -button_height))) {
        namespace fs = std::filesystem;
        for (std::size_t i = 0; i < state.scan_paths.size(); ++i) {
            auto const& p = state.scan_paths[i];
            std::error_code ec;
            auto rel = fs::relative(p, state.folder, ec);
            std::string label = (ec || rel.empty()) ? p.string() : rel.string();

            ImGui::PushID(static_cast<int>(i));
            bool is_selected = state.selected[i];
            if (ImGui::Selectable(label.c_str(), is_selected)) {
                state.selected[i] = !is_selected;
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
}

bool add_instruments_from_folder_dialog::import_footer(scan_state& state) {
    bool close = false;
    if (!state.have_paths()) {
        ImGui::BeginDisabled();
    }
    bool const import_clicked = ImGui::Button("Import");
    if (!state.have_paths()) {
        ImGui::EndDisabled();
    }
    if (import_clicked) {
        std::vector<std::string> paths;
        paths.reserve(state.scan_paths.size());
        bool const filter = state.selected_count() > 0;
        for (std::size_t i = 0; i < state.scan_paths.size(); ++i) {
            if (!filter || state.selected[i]) {
                paths.push_back(state.scan_paths[i].string());
            }
        }
        handler_.emit<event::add_instruments>(std::move(paths));
        ImGui::CloseCurrentPopup();
        close = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        ImGui::CloseCurrentPopup();
        close = true;
    }
    return close;
}

bool add_instruments_from_folder_dialog::draw() {
    task_.tick();
    return true;
}

}

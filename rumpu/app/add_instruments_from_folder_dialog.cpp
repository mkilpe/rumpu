
#include "add_instruments_from_folder_dialog.hpp"
#include "events.hpp"
#include "native_file_dialog.hpp"
#include "dialog_widgets.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include <algorithm>
#include <filesystem>
#include <thread>
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
    struct scan_result {
        std::vector<std::filesystem::path> paths;
        std::string error;
    };

    std::size_t selected_count() const {
        return static_cast<std::size_t>(std::count(selected.begin(), selected.end(), true));
    }
    bool have_paths() const { return !scan_paths.empty() && scan_error.empty(); }
    bool scanning() const { return result->in_flight(); }

    std::string folder;
    bool recursive = false;

    std::string scanned_folder;
    bool scanned_recursive = false;
    std::vector<std::filesystem::path> scan_paths;
    // one display label per scanned path, computed when a scan completes so
    // drawing the list costs no filesystem work
    std::vector<std::string> labels;
    std::vector<bool> selected;
    std::string scan_error;
    // scanning happens after the folder text has been stable for a moment, so
    // typing a path does not hit the filesystem on every keystroke
    double changed_time = -1.0;
    bool scan_now = false;
    // scans run on a worker thread and deliver here; a superseded scan's
    // delivery is dropped by the session token
    std::shared_ptr<async_result<scan_result>> result = std::make_shared<async_result<scan_result>>();
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

    browse_button("Browse...", folder_result_, open_folder_dialog);
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
            start_scan(state);
        }
    } else {
        state.changed_time = -1.0;
    }
    state.scan_now = false;

    if (auto res = state.result->take()) {
        state.scan_paths = std::move(res->paths);
        state.scan_error = std::move(res->error);
        state.selected.assign(state.scan_paths.size(), false);
        state.labels.clear();
        state.labels.reserve(state.scan_paths.size());
        for (auto const& p : state.scan_paths) {
            state.labels.push_back(p.lexically_proximate(state.scanned_folder).string());
        }
    }
}

// Hand the directory walk to a worker thread: a large or slow (network)
// folder must not stall the frame loop. The previous results are cleared so
// an import cannot act on the old folder's list while the scan runs.
void add_instruments_from_folder_dialog::start_scan(scan_state& state) {
    state.scanned_folder = state.folder;
    state.scanned_recursive = state.recursive;
    state.changed_time = -1.0;
    state.scan_paths.clear();
    state.labels.clear();
    state.selected.clear();
    state.scan_error.clear();

    state.result->invalidate(); // supersede a scan still running
    if (auto session = state.result->begin()) {
        std::thread{[m = state.result, s = *session,
                folder = state.folder, recursive = state.recursive] {
            scan_state::scan_result r;
            scan_folder(folder, recursive, r.paths, r.error);
            m->deliver(s, std::move(r));
        }}.detach();
    }
}

void add_instruments_from_folder_dialog::selection_controls(scan_state& state) {
    if (!state.scan_error.empty()) {
        ImGui::TextColored({1.0f, 0.5f, 0.5f, 1.0f}, "Error: %s", state.scan_error.c_str());
    } else if (state.scanning()) {
        ImGui::TextDisabled("Scanning...");
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
        for (std::size_t i = 0; i < state.scan_paths.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            bool is_selected = state.selected[i];
            if (ImGui::Selectable(state.labels[i].c_str(), is_selected)) {
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

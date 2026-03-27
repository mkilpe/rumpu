
#include "add_instrument_dialog.hpp"
#include "events.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

namespace securepath::drum::app {

add_instrument_dialog::add_instrument_dialog(event_system::event_handler& h)
: handler_(h)
{}

static std::string filename_stem(std::string const& path) {
    auto pos = path.find_last_of("/\\");
    std::string filename = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    auto dot = filename.rfind('.');
    return (dot != std::string::npos) ? filename.substr(0, dot) : filename;
}

void add_instrument_dialog::open() {
    task_ = run();
}

std::string add_instrument_dialog::collect_file_result() {
    std::lock_guard l{result_mutex_};
    if (has_result_) {
        has_result_ = false;
        return std::move(result_path_);
    }
    return {};
}

ui_task add_instrument_dialog::run() {
    ImGui::OpenPopup("Add instrument");
    co_await next_frame{};

    std::string path;
    std::string name;
    bool browsing = false;

    while (true) {
        if (auto result = collect_file_result(); !result.empty()) {
            path = std::move(result);
            if (name.empty()) {
                name = filename_stem(path);
            }
            browsing = false;
        }

        if (!ImGui::BeginPopupModal("Add instrument", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        ImGui::Text("WAV file path:");
        float browse_width = ImGui::CalcTextSize("Browse...").x + ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetNextItemWidth(-browse_width);
        ImGui::InputText("##path", &path);
        ImGui::SameLine();

        bool const was_browsing = browsing;
        if (was_browsing) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Browse...")) {
            browsing = true;
            open_wav_file_dialog([this](std::string p) {
                std::lock_guard l{result_mutex_};
                result_path_ = std::move(p);
                has_result_ = true;
            });
        }
        if (was_browsing) {
            ImGui::EndDisabled();
        }

        ImGui::Text("Name:");
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##name", &name);

        if (ImGui::Button("Add")) {
            handler_.emit<event::add_instrument>(path, name);
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

bool add_instrument_dialog::draw() {
    task_.tick();
    return true;
}

}

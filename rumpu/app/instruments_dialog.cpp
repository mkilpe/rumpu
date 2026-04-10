
#include "instruments_dialog.hpp"
#include "native_file_dialog.hpp"

#include "imgui.h"
#include "imgui_stdlib.h"

#include <filesystem>
#include <string>

namespace securepath::drum::app {

void instruments_dialog::open(song* s, undo_manager* undo) {
    task_ = run(s, undo);
}

std::string instruments_dialog::collect_file_result() {
    std::lock_guard l{result_mutex_};
    if (has_result_) {
        has_result_ = false;
        return std::move(result_path_);
    }
    return {};
}

static std::string sample_label(drum_sample const& s) {
    namespace fs = std::filesystem;
    auto path = s.source_file();
    if (path.empty()) {
        return "<empty>";
    }
    return fs::path{path}.filename().string();
}

ui_task instruments_dialog::run(song* s, undo_manager* undo) {
    if (undo) {
        undo->snapshot(*s);
    }

    ImGui::OpenPopup("Instruments");
    co_await next_frame{};

    int selected = s->instruments().empty() ? -1 : 0;
    int selected_sample = -1;
    bool browsing = false;

    while (true) {
        if (auto result = collect_file_result(); !result.empty()) {
            if (selected >= 0 && static_cast<std::size_t>(selected) < s->instruments().size()) {
                s->instruments()[selected].add_sample(std::move(result));
            }
            browsing = false;
        }

        ImGui::SetNextWindowSize({720, 480}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Instruments", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        auto& instruments = s->instruments();
        if (selected >= static_cast<int>(instruments.size())) {
            selected = instruments.empty() ? -1 : 0;
            selected_sample = -1;
        }

        float const button_row_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
        ImVec2 const avail = ImGui::GetContentRegionAvail();
        float const left_width = 220.0f;

        ImGui::BeginChild("##left", ImVec2(left_width, avail.y - button_row_height), ImGuiChildFlags_Border);
        for (std::size_t i = 0; i < instruments.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            bool is_sel = (static_cast<int>(i) == selected);
            if (ImGui::Selectable(instruments[i].name().c_str(), is_sel)) {
                selected = static_cast<int>(i);
                selected_sample = -1;
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##right", ImVec2(0, avail.y - button_row_height), ImGuiChildFlags_Border);
        if (selected < 0 || static_cast<std::size_t>(selected) >= instruments.size()) {
            ImGui::TextDisabled("No instrument selected.");
        } else {
            auto& inst = instruments[selected];

            ImGui::Text("Name:");
            ImGui::SetNextItemWidth(-FLT_MIN);
            std::string name = inst.name();
            if (ImGui::InputText("##name", &name)) {
                inst.set_name(std::move(name));
            }

            ImGui::Separator();
            ImGui::Text("Volume:");
            auto vol = inst.volume();
            float vol_value = static_cast<float>(vol.value);
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::SliderFloat("##volume", &vol_value, 0.0f, 1.0f, "%.2f")) {
                vol.value = vol_value;
                inst.set_volume(vol);
            }
            bool mute = vol.mute;
            if (ImGui::Checkbox("Mute", &mute)) {
                vol.mute = mute;
                inst.set_volume(vol);
            }

            ImGui::Separator();
            ImGui::Text("Samples:");

            float const sample_button_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
            if (ImGui::BeginListBox("##samples", ImVec2(-FLT_MIN, -sample_button_height))) {
                auto const& samples = inst.samples();
                for (std::size_t i = 0; i < samples.size(); ++i) {
                    ImGui::PushID(static_cast<int>(i));
                    bool is_sel = (static_cast<int>(i) == selected_sample);
                    auto label = sample_label(samples[i]);
                    if (ImGui::Selectable(label.c_str(), is_sel)) {
                        selected_sample = static_cast<int>(i);
                    }
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("%s", samples[i].source_file().c_str());
                    }
                    ImGui::PopID();
                }
                ImGui::EndListBox();
            }

            bool const was_browsing = browsing;
            if (was_browsing) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Add sample...")) {
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

            ImGui::SameLine();

            bool const can_remove = selected_sample >= 0
                && static_cast<std::size_t>(selected_sample) < inst.samples().size()
                && inst.samples().size() > 1;
            if (!can_remove) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Remove sample")) {
                inst.remove_sample(static_cast<std::size_t>(selected_sample));
                if (selected_sample >= static_cast<int>(inst.samples().size())) {
                    selected_sample = inst.samples().empty()
                        ? -1
                        : static_cast<int>(inst.samples().size()) - 1;
                }
            }
            if (!can_remove) {
                ImGui::EndDisabled();
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }

        ImGui::EndPopup();
        co_await next_frame{};
    }
}

bool instruments_dialog::draw() {
    task_.tick();
    return true;
}

}

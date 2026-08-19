
#include "instruments_dialog.hpp"
#include "native_file_dialog.hpp"
#include <rumpu/core/song_edit.hpp>
#include <rumpu/core/song_file.hpp>

#include "imgui.h"
#include "imgui_stdlib.h"

#include <filesystem>
#include <string>

namespace securepath::drum::app {

void instruments_dialog::open(song* s, undo_manager* undo, std::filesystem::path project_base) {
    // Drop a result delivered after the previous dialog instance closed
    file_result_->take();
    task_ = run(s, undo, std::move(project_base));
}

static std::string sample_label(drum_sample const& s) {
    namespace fs = std::filesystem;
    auto path = s.source_file();
    if (path.empty()) {
        return "<empty>";
    }
    return fs::path{path}.filename().string();
}

ui_task instruments_dialog::run(song* s, undo_manager* undo, std::filesystem::path project_base) {
    ImGui::OpenPopup("Instruments");
    co_await next_frame{};

    int selected = s->instruments().empty() ? -1 : 0;
    int selected_sample = -1;

    while (true) {
        handle_added_sample(*s, undo, project_base, selected);

        ImGui::SetNextWindowSize({720, 480}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Instruments", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        panels(*s, undo, selected, selected_sample);

        if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }

        ImGui::EndPopup();
        co_await next_frame{};
    }
}

void instruments_dialog::handle_added_sample(song& s, undo_manager* undo,
    std::filesystem::path const& project_base, int selected) {
    if (auto result = file_result_->take(); result && !result->empty()) {
        if (selected >= 0 && static_cast<std::size_t>(selected) < s.instruments().size()) {
            song_edit edit{s, undo};
            // store project-relative like the add-instrument path, so
            // projects stay relocatable
            s.instruments()[selected].add_sample(project_relative_path(*result, project_base));
        }
    }
}

void instruments_dialog::panels(song& s, undo_manager* undo, int& selected, int& selected_sample) {
    auto& instruments = s.instruments();
    if (selected >= static_cast<int>(instruments.size())) {
        selected = instruments.empty() ? -1 : 0;
        selected_sample = -1;
    }

    float const button_row_height = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
    ImVec2 const avail = ImGui::GetContentRegionAvail();

    ImGui::BeginChild("##left", ImVec2(220.0f, avail.y - button_row_height), ImGuiChildFlags_Border);
    instrument_list(s, selected, selected_sample);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##right", ImVec2(0, avail.y - button_row_height), ImGuiChildFlags_Border);
    if (selected < 0 || static_cast<std::size_t>(selected) >= instruments.size()) {
        ImGui::TextDisabled("No instrument selected.");
    } else {
        auto& inst = instruments[selected];
        instrument_properties(s, undo, inst);
        sample_list(inst, selected_sample);
        sample_buttons(s, undo, inst, selected_sample);
    }
    ImGui::EndChild();
}

void instruments_dialog::instrument_list(song& s, int& selected, int& selected_sample) {
    auto& instruments = s.instruments();
    for (std::size_t i = 0; i < instruments.size(); ++i) {
        ImGui::PushID(static_cast<int>(i));
        bool is_sel = (static_cast<int>(i) == selected);
        if (ImGui::Selectable(instruments[i].name().c_str(), is_sel)) {
            selected = static_cast<int>(i);
            selected_sample = -1;
        }
        ImGui::PopID();
    }
}

void instruments_dialog::instrument_properties(song& s, undo_manager* undo, instrument& inst) {
    ImGui::Text("Name:");
    ImGui::SetNextItemWidth(-FLT_MIN);
    std::string name = inst.name();
    if (ImGui::InputText("##name", &name)) {
        song_edit edit{s, undo, coalesce_key::instrument_edit};
        inst.set_name(std::move(name));
    }

    ImGui::Separator();
    ImGui::Text("Volume:");
    auto vol = inst.volume();
    float vol_value = static_cast<float>(vol.value);
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::SliderFloat("##volume", &vol_value, 0.0f, 1.0f, "%.2f")) {
        vol.value = vol_value;
        song_edit edit{s, undo, coalesce_key::instrument_edit};
        inst.set_volume(vol);
    }
    bool mute = vol.mute;
    if (ImGui::Checkbox("Mute", &mute)) {
        vol.mute = mute;
        song_edit edit{s, undo};
        inst.set_volume(vol);
    }
}

void instruments_dialog::sample_list(instrument const& inst, int& selected_sample) {
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
}

void instruments_dialog::sample_buttons(song& s, undo_manager* undo, instrument& inst, int& selected_sample) {
    bool const browsing = file_result_->in_flight();
    if (browsing) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Add sample...") && file_result_->begin()) {
        open_wav_file_dialog([r = file_result_](std::string p) {
            r->deliver(std::move(p));
        });
    }
    if (browsing) {
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
        {
            song_edit edit{s, undo};
            inst.remove_sample(static_cast<std::size_t>(selected_sample));
        }
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

bool instruments_dialog::draw() {
    task_.tick();
    return true;
}

}

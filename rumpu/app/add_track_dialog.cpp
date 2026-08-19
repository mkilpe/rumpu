
#include "add_track_dialog.hpp"
#include "events.hpp"

#include "imgui.h"

#include <filesystem>
#include <format>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace securepath::drum::app {

static std::vector<std::string> make_instrument_labels(std::vector<instrument> const& instruments) {
    namespace fs = std::filesystem;

    std::unordered_map<std::string_view, int> name_counts;
    for (auto const& inst : instruments) {
        ++name_counts[inst.name()];
    }

    std::vector<std::string> labels(instruments.size());
    std::unordered_map<std::string, int> label_counts;

    for (std::size_t i = 0; i < instruments.size(); ++i) {
        auto const& inst = instruments[i];
        if (name_counts[inst.name()] <= 1) {
            labels[i] = inst.name();
        } else {
            auto parent = fs::path{inst.sample_to_play().source_file()}
                .parent_path().filename().string();
            labels[i] = parent.empty()
                ? inst.name()
                : std::format("{} ({})", inst.name(), parent);
        }
        ++label_counts[labels[i]];
    }

    for (std::size_t i = 0; i < instruments.size(); ++i) {
        if (label_counts[labels[i]] > 1) {
            auto const& src = instruments[i].sample_to_play().source_file();
            labels[i] = std::format("{} ({})", instruments[i].name(), src);
        }
    }

    return labels;
}

add_track_dialog::add_track_dialog(event_system::event_handler& h)
: handler_(h)
{}

void add_track_dialog::open(song* s, std::uint32_t section) {
    task_ = run(s, section);
}

ui_task add_track_dialog::run(song* s, std::uint32_t section) {
    ImGui::OpenPopup("Add track");
    co_await next_frame{};

    int selected = -1;

    while (true) {
        ImGui::SetNextWindowSize({380, 420}, ImGuiCond_FirstUseEver);
        if (!ImGui::BeginPopupModal("Add track", nullptr, ImGuiWindowFlags_None)) {
            co_return;
        }

        instrument_list(*s, selected);

        bool const has_selection = selected >= 0 && static_cast<std::size_t>(selected) < s->instruments().size();
        if (!has_selection) {
            ImGui::BeginDisabled();
        }
        bool const do_add = ImGui::Button("Add");
        if (!has_selection) {
            ImGui::EndDisabled();
        }
        if (do_add) {
            handler_.emit<event::add_track>(section, static_cast<std::size_t>(selected));
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

void add_track_dialog::instrument_list(song const& s, int& selected) {
    auto const& instruments = s.instruments();
    auto labels = make_instrument_labels(instruments);

    float button_height = ImGui::GetFrameHeightWithSpacing();
    if (ImGui::BeginListBox("##instruments", ImVec2(-FLT_MIN, -button_height))) {
        for (std::size_t i = 0; i < instruments.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            bool is_selected = (static_cast<int>(i) == selected);
            if (ImGui::Selectable(labels[i].c_str(), is_selected)) {
                selected = static_cast<int>(i);
            }
            ImGui::PopID();
        }
        ImGui::EndListBox();
    }
}

bool add_track_dialog::draw() {
    task_.tick();
    return true;
}

}

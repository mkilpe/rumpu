
#include "section_view.hpp"
#include "events.hpp"

#include <securepath/log/log.hpp>
#include "imgui.h"

#include <string>

namespace securepath::drum::app {

section_view::section_view(event_system::event_handler& h)
: child_window_base("section_view", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
, handler_(h)
{}

void section_view::set_context(song* s, uint32_t section) {
    song_ = s;
    current_section_ = section;
}

bool section_view::do_draw() {
    if (!song_) {
        return false;
    }

    auto& order = song_->section_order();
    auto const& sections = song_->sections();

    if (ImGui::Button("+##add_section")) {
        LOG_TRACE("add_section clicked");
        handler_.emit<event::add_section>();
    }

    // Draw section order buttons with drag-and-drop reordering
    int drag_source = -1;
    int drag_target = -1;

    auto const& style = ImGui::GetStyle();
    float max_x = ImGui::GetContentRegionMax().x;

    for (std::size_t i = 0; i < order.size(); ++i) {
        auto id = order[i];
        bool is_selected = (id == current_section_);

        // Use section name if available
        auto it = sections.find(id);
        std::string name = (it != sections.end() && !it->second.name().empty())
            ? it->second.name()
            : "Section " + std::to_string(id);
        std::string label = std::to_string(i + 1) + ": " + name + "##order_" + std::to_string(i);

        // Wrap to next row if button doesn't fit
        float button_w = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2;
        float last_x = ImGui::GetItemRectMax().x;
        float next_x = last_x + style.ItemSpacing.x + button_w;
        float window_x = ImGui::GetWindowPos().x + max_x;
        if (next_x < window_x) {
            ImGui::SameLine();
        }

        if (is_selected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }

        if (ImGui::Button(label.c_str())) {
            handler_.emit<event::select_section>(id);
        }

        // Right-click context menu
        std::string ctx_id = "section_ctx_" + std::to_string(i);
        if (ImGui::BeginPopupContextItem(ctx_id.c_str())) {
            if (ImGui::Selectable("Duplicate in queue")) {
                order.insert(order.begin() + static_cast<std::ptrdiff_t>(i) + 1, id);
            }
            if (ImGui::Selectable("Remove from queue")) {
                order.erase(order.begin() + static_cast<std::ptrdiff_t>(i));
            }
            ImGui::EndPopup();
        }

        // Drag source
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            int idx = static_cast<int>(i);
            ImGui::SetDragDropPayload("SECTION_ORDER", &idx, sizeof(int));
            ImGui::Text("%s", name.c_str());
            ImGui::EndDragDropSource();
        }

        // Drop target
        if (ImGui::BeginDragDropTarget()) {
            if (auto const* payload = ImGui::AcceptDragDropPayload("SECTION_ORDER")) {
                drag_source = *static_cast<int const*>(payload->Data);
                drag_target = static_cast<int>(i);
            }
            ImGui::EndDragDropTarget();
        }

        if (is_selected) {
            ImGui::PopStyleColor();
        }
    }

    // Invisible drop target after the last button so items can be dragged to the end
    if (!order.empty()) {
        ImGui::SameLine();
        ImGui::InvisibleButton("##drop_end", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()));
        if (ImGui::BeginDragDropTarget()) {
            if (auto const* payload = ImGui::AcceptDragDropPayload("SECTION_ORDER")) {
                drag_source = *static_cast<int const*>(payload->Data);
                drag_target = static_cast<int>(order.size() - 1);
            }
            ImGui::EndDragDropTarget();
        }
    }

    // Apply reorder after the loop to avoid mutating during iteration
    if (drag_source >= 0 && drag_target >= 0 && drag_source != drag_target) {
        auto val = order[drag_source];
        order.erase(order.begin() + drag_source);
        order.insert(order.begin() + drag_target, val);
    }

    return true;
}

}


#include "section_view.hpp"
#include "events.hpp"
#include <rumpu/core/undo_manager.hpp>
#include <rumpu/core/song_edit.hpp>

#include <securepath/log/log.hpp>
#include "imgui.h"

#include <string>

namespace securepath::drum::app {

section_view::section_view(event_system::event_handler& h)
: child_window_base("section_view", ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse, {})
, handler_(h)
{}

void section_view::set_context(song* s, uint32_t section, undo_manager* undo) {
    undo_ = undo;
    song_ = s;
    current_section_ = section;
}

struct section_view::drag_move {
    bool valid() const { return source >= 0 && target >= 0 && source != target; }

    int source = -1;
    int target = -1;
};

bool section_view::do_draw() {
    if (!song_) {
        return false;
    }

    if (ImGui::Button("+##add_section")) {
        LOG_TRACE("add_section clicked");
        handler_.emit<event::add_section>();
    }

    // Draw section order buttons with drag-and-drop reordering; the move is
    // applied after the loop to avoid mutating the order during iteration
    drag_move move;
    for (std::size_t i = 0; i < song_->section_order().size(); ++i) {
        section_button(i, move);
    }
    end_drop_target(move);

    if (move.valid()) {
        song_edit edit{*song_, undo_};
        auto& order = song_->section_order();
        auto val = order[move.source];
        order.erase(order.begin() + move.source);
        order.insert(order.begin() + move.target, val);
    }

    return true;
}

void section_view::section_button(std::size_t i, drag_move& move) {
    auto& order = song_->section_order();
    auto const& sections = song_->sections();
    auto const id = order[i];
    bool const is_selected = (id == current_section_);

    // Use section name if available
    auto it = sections.find(id);
    std::string name = (it != sections.end() && !it->second.name().empty())
        ? it->second.name()
        : "Section " + std::to_string(id);
    std::string label = std::to_string(i + 1) + ": " + name + "##order_" + std::to_string(i);

    // Wrap to next row if button doesn't fit
    auto const& style = ImGui::GetStyle();
    float button_w = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2;
    float next_x = ImGui::GetItemRectMax().x + style.ItemSpacing.x + button_w;
    if (next_x < ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x) {
        ImGui::SameLine();
    }

    if (is_selected) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    }
    if (ImGui::Button(label.c_str())) {
        handler_.emit<event::select_section>(id);
    }

    queue_context_menu(i, id);

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        int idx = static_cast<int>(i);
        ImGui::SetDragDropPayload("SECTION_ORDER", &idx, sizeof(int));
        ImGui::Text("%s", name.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (auto const* payload = ImGui::AcceptDragDropPayload("SECTION_ORDER")) {
            move.source = *static_cast<int const*>(payload->Data);
            move.target = static_cast<int>(i);
        }
        ImGui::EndDragDropTarget();
    }

    if (is_selected) {
        ImGui::PopStyleColor();
    }
}

void section_view::queue_context_menu(std::size_t i, std::uint32_t id) {
    auto& order = song_->section_order();
    std::string ctx_id = "section_ctx_" + std::to_string(i);
    if (ImGui::BeginPopupContextItem(ctx_id.c_str())) {
        if (ImGui::Selectable("Duplicate in queue")) {
            song_edit edit{*song_, undo_};
            order.insert(order.begin() + static_cast<std::ptrdiff_t>(i) + 1, id);
        }
        if (ImGui::Selectable("Remove from queue")) {
            song_edit edit{*song_, undo_};
            order.erase(order.begin() + static_cast<std::ptrdiff_t>(i));
        }
        ImGui::EndPopup();
    }
}

// Invisible drop target after the last button so items can be dragged to the end
void section_view::end_drop_target(drag_move& move) {
    auto const& order = song_->section_order();
    if (!order.empty()) {
        ImGui::SameLine();
        ImGui::InvisibleButton("##drop_end", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight()));
        if (ImGui::BeginDragDropTarget()) {
            if (auto const* payload = ImGui::AcceptDragDropPayload("SECTION_ORDER")) {
                move.source = *static_cast<int const*>(payload->Data);
                move.target = static_cast<int>(order.size() - 1);
            }
            ImGui::EndDragDropTarget();
        }
    }
}

}

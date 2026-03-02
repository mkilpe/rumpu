
#include "about_dialog.hpp"
#include "license.hpp"

#include "imgui.h"

namespace securepath::drum::app {

void about_dialog::open() {
    open_ = true;
}

void about_dialog::do_draw() {
    if (open_) {
        ImGui::OpenPopup("About Rumpu");
        open_ = false;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({500, 300}, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal("About Rumpu", nullptr)) {
        ImGui::SeparatorText("License");
        ImGui::InputTextMultiline("##license", const_cast<char*>(license_text), sizeof(license_text),
            ImVec2(-1, -ImGui::GetFrameHeightWithSpacing()), ImGuiInputTextFlags_ReadOnly);
        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

}


#include "about_dialog.hpp"
#include "license.hpp"
#include "version.hpp"

#include "imgui.h"

namespace securepath::drum::app {

static bool begin_about_popup() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, {0.5f, 0.5f});
    ImGui::SetNextWindowSize({560, 400}, ImGuiCond_FirstUseEver);
    return ImGui::BeginPopupModal("About Rumpu", nullptr);
}

void about_dialog::open() {
    task_ = run();
}

ui_task about_dialog::run() {
    ImGui::OpenPopup("About Rumpu");
    co_await next_frame{};

    while (true) {
        if (!begin_about_popup()) {
            co_return;
        }
        ImGui::Text("Rumpu %s", version);
        ImGui::SeparatorText("License");
        ImGui::InputTextMultiline("##license", const_cast<char*>(license_text), sizeof(license_text),
            ImVec2(-1, -ImGui::GetFrameHeightWithSpacing()), ImGuiInputTextFlags_ReadOnly);
        if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            co_return;
        }
        ImGui::EndPopup();
        co_await next_frame{};
    }
}

bool about_dialog::draw() {
    task_.tick();
    return true;
}

}


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

static void license_tab(char const* label, char const* text) {
    if (ImGui::BeginTabItem(label)) {
        ImGui::BeginChild("##text", {0, -ImGui::GetFrameHeightWithSpacing()},
            ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::TextUnformatted(text);
        ImGui::EndChild();
        ImGui::EndTabItem();
    }
}

ui_task about_dialog::run() {
    ImGui::OpenPopup("About Rumpu");
    co_await next_frame{};

    while (true) {
        if (!begin_about_popup()) {
            co_return;
        }
        ImGui::Text("Rumpu %s", version);
        if (ImGui::BeginTabBar("##about_tabs")) {
            license_tab("License", license_text);
            license_tab("Third-party licenses", third_party_licenses_text);
            ImGui::EndTabBar();
        }
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

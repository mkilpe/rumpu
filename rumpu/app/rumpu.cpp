
#include "rumpu.hpp"

#include "track_list.hpp"

#include "imgui.h"

namespace securepath::drum {

rumpu::rumpu() : event_handler(static_cast<securepath::event_system::event_loop&>(*this)) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    windows_.push_back(child_window_ptr(new view_child_window("TrackList", view_ptr(new track_list()))));
}

void rumpu::menu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Close"))  { 
                running_ = false; 
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Views")) {
            for(auto&& w : windows_) {
                if (ImGui::MenuItem(w->name().c_str(), nullptr, w->is_visible()))  {
                    w->set_visible(!w->is_visible());
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Settings")) {

            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

bool rumpu::update() {
    std::unique_lock l{mutex_};

#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);
#else 
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
   
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("Rumpu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar);
    
    menu();

    for(auto&& w : windows_) {
        w->draw();
    }

    ImGui::End();
    ImGui::PopStyleVar(1);
    
    return running_;
}

void rumpu::handle_event(std::unique_ptr<securepath::event_system::event_base> ev) {
    //dispatch(*ev        
        //, event_dest<on_price_data>(&rumpu::on_price)
    //    );
}

}

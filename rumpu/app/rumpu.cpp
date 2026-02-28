
#include "rumpu.hpp"

#include "events.hpp"
#include "toolbar.hpp"
#include <securepath/log/log.hpp>

#include "imgui.h"

namespace securepath::drum::app {

rumpu::rumpu()
: event_handler(static_cast<securepath::event_system::event_loop&>(*this))
, add_instrument_dialog_(*this)
, song_({}, {3,4}, {60.0})
, player_(*this)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    //for testing
    song_.add_instrument(instrument{"test_kick.wav"});
    auto id = song_.add_section();
    song_.section_order().push_back(id);

    if(auto section = song_.find_section(id)) {
        section->set_length(10);
        auto& tracks = section->tracks()[0];
        size_t i = 0;
        for(auto&& b : tracks.bars()) {
            if(++i % 4 == 0) {
                for(std::size_t count = 0; count != song_.default_time_signature().beats_in_bar(); ++count) {
                    b.beats.push_back({beat::hit});
                }
            } else {
                b.beats.push_back({beat::hit});
            }        
        }
    }

    //if(auto section = song_.find_section(id)) {
    //    section->tracks()[0];
    //}

    track_edit_view_.reset(new track_edit_view(*this));
    windows_.push_back(track_edit_view_.get());

    current_section_ = id;
    track_edit_view_->set_context(&song_, id);
}

void rumpu::menu() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open..."))  { 
            }
            if (ImGui::MenuItem("Save"))  { 
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Close"))  { 
                running_ = false; 
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Sections")) {
            for (auto const& [id, section] : song_.sections()) {
                std::string label = "Section " + std::to_string(id);
                bool is_selected = (id == current_section_);
                if (ImGui::MenuItem(label.c_str(), nullptr, is_selected)) {
                    select_section_impl(id);
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Views")) {
            for(auto&& w : windows_) {
                if (ImGui::MenuItem(w->name().c_str(), nullptr, w->is_visible())) {
                    w->set_visible(!w->is_visible());
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Instruments")) {
            if (ImGui::MenuItem("Add instrument...")) {
                add_instrument_dialog_.open();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            if (ImGui::MenuItem("Settings")) {

            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::Separator();
            if (ImGui::MenuItem("About")) {
                show_about();
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
    add_instrument_dialog_.do_draw();

    if(!windows_.empty()) {
        auto pos = ImGui::GetCursorPos();
        auto size = ImGui::GetContentRegionAvail();
        size.y /= windows_.size();

        std::size_t i = 0;
        for(auto&& w : windows_) {
            pos.y += size.y*i;
            ImGui::SetNextWindowPos(pos);
            ImGui::SetNextWindowSize(size);
            w->draw();
        }
    }

    ImGui::End();
    ImGui::PopStyleVar(1);
    
    return running_;
}

void rumpu::add_track(uint32_t section) {
    std::unique_lock l{mutex_};
    LOG_TRACE("add_track");
    // for testing
    song_.add_instrument(instrument{"test_kick.wav"});
    track_edit_view_->set_context(&song_, section);
}

void rumpu::select_section_impl(uint32_t section_id) {
    current_section_ = section_id;
    track_edit_view_->set_context(&song_, section_id);
    player_pos_changed();
}

void rumpu::select_section(uint32_t section_id) {
    std::unique_lock l{mutex_};
    select_section_impl(section_id);
}

void rumpu::add_instrument(std::string path) {
    std::unique_lock l{mutex_};
    song_.add_instrument(instrument{path});
    track_edit_view_->set_context(&song_, current_section_);
}

void rumpu::add_section() {
    std::unique_lock l{mutex_};
    LOG_TRACE("add_section");
    auto id = song_.add_section();
    song_.section_order().push_back(id);
    current_section_ = id;
    track_edit_view_->set_context(&song_, id);
}

void rumpu::play_song(uint32_t section) {
    player_.play(&song_, false, section);
}

void rumpu::stop_song(uint32_t section) {
    player_.stop();
}

void rumpu::player_pos_changed() {
    LOG_TRACE("play pos changed");
    track_edit_view_->set_play_status(player_.get_status());
}

void rumpu::handle_event(std::unique_ptr<securepath::event_system::event_base> ev) {
    dispatch(*ev
        , event_dest<event::add_track>(&rumpu::add_track)
        , event_dest<event::add_instrument>(&rumpu::add_instrument)
        , event_dest<event::play_song>(&rumpu::play_song)
        , event_dest<event::stop_song>(&rumpu::stop_song)
        , event_dest<event::select_section>(&rumpu::select_section)
        , event_dest<event::add_section>(&rumpu::add_section)
        , event_dest<drum::event::player_pos_changed>(&rumpu::player_pos_changed)
        );
}

void rumpu::show_about() const
{
    if (ImGui::Begin("About", &show_about_, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("some");
        ImGui::End();
    }
}

}

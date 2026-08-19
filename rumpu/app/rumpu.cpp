
#include "rumpu.hpp"

#include "events.hpp"
#include "native_file_dialog.hpp"
#include "toolbar.hpp"
#include <rumpu/core/export.hpp>
#include <rumpu/core/song_file.hpp>
#include <rumpu/core/song_edit.hpp>
#include <securepath/log/log.hpp>

#include "imgui.h"

#include <format>
#include <utility>
#include <vector>

namespace securepath::drum::app {

static std::filesystem::path project_dir(std::string const& file) {
    return file.empty() ? std::filesystem::path{} : std::filesystem::path{file}.parent_path();
}

rumpu::rumpu(app_options options)
: event_handler(static_cast<securepath::event_system::event_loop&>(*this))
, add_instrument_dialog_(*this)
, add_instruments_from_folder_dialog_(*this)
, add_track_dialog_(*this)
, new_song_dialog_(*this)
, song_properties_dialog_(*this)
, song_({"Untitled", "", ""}, {4,4}, {120.0})
, player_(*this)
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();

    auto id = song_.add_section();
    song_.section_order().push_back(id);

    track_edit_view_.reset(new track_edit_view(*this));
    windows_.push_back(track_edit_view_.get());

    current_section_ = id;
    track_edit_view_->set_context(&song_, id, &undo_);

    if (!options.initial_file.empty()) {
        open_project(std::move(options.initial_file));
    }
}

rumpu::~rumpu() {
    // Stop receiving events before our members are destroyed. The event-loop
    // thread lives in a base class destroyed after us, so without this it could
    // dispatch a queued event (e.g. player_pos_changed) into already-destroyed
    // members during shutdown.
    stop_handler();
}

void rumpu::perform_undo() {
    std::unique_lock l{song_.mutex};
    if (undo_.undo(song_)) {
        if (!song_.find_section(current_section_)) {
            if (!song_.sections().empty()) {
                current_section_ = song_.sections().begin()->first;
            }
        }
        track_edit_view_->set_context(&song_, current_section_, &undo_);
    }
}

void rumpu::perform_redo() {
    std::unique_lock l{song_.mutex};
    if (undo_.redo(song_)) {
        if (!song_.find_section(current_section_)) {
            if (!song_.sections().empty()) {
                current_section_ = song_.sections().begin()->first;
            }
        }
        track_edit_view_->set_context(&song_, current_section_, &undo_);
    }
}

void rumpu::open_project_dialog(project_action action) {
    // One dialog in flight at a time; the callback shares ownership of the
    // mailbox only, so a result delivered after shutdown is dropped harmlessly.
    if (!project_dialog_result_->begin()) {
        return;
    }
    pending_project_action_ = action;
    auto callback = [r = project_dialog_result_](std::string path) {
        r->deliver(std::move(path));
    };
    if (action == project_action::open) {
        open_project_file_dialog(std::move(callback));
    } else {
        save_project_file_dialog(std::move(callback));
    }
}

void rumpu::poll_project_dialog_result() {
    auto path = project_dialog_result_->take();
    if (!path) {
        return;
    }
    auto action = std::exchange(pending_project_action_, project_action::none);
    if (path->empty()) {
        return;
    }
    if (action == project_action::open) {
        event_system::event_handler::emit<event::open_project>(std::move(*path));
    } else if (action == project_action::save) {
        event_system::event_handler::emit<event::save_project>(std::move(*path));
    }
}

void rumpu::menu() {
    if (ImGui::BeginMenuBar()) {
        file_menu();
        edit_menu();
        sections_menu();
        other_menus();
        ImGui::EndMenuBar();
    }
}

void rumpu::save_current_file() {
    try {
        save_song_file(current_file_, song_);
    } catch(std::exception const& e) {
        LOG_WARN("save_song_file failed: {}", e.what());
        show_error(std::format("Failed to save project: {}", e.what()));
    }
}

void rumpu::open_export_dialog() {
    // Export from a snapshot loaded at the export rate; the live
    // song stays at the player rate so playback is unaffected.
    try {
        song copy{song_};
        copy.load_instruments(export_options{}.format.samples_per_second, project_dir(current_file_));
        export_dialog_.open(std::move(copy));
    } catch(std::exception const& e) {
        LOG_WARN("load_instruments failed: {}", e.what());
        show_error(std::format("Failed to load instruments: {}", e.what()));
    }
}

void rumpu::file_menu() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Song")) {
            new_song_dialog_.open();
        }
        if (ImGui::MenuItem("Open...")) {
            open_project_dialog(project_action::open);
        }
        if (ImGui::MenuItem("Save")) {
            if (!current_file_.empty()) {
                save_current_file();
            } else {
                open_project_dialog(project_action::save);
            }
        }
        if (ImGui::MenuItem("Save As...")) {
            open_project_dialog(project_action::save);
        }
        if (ImGui::MenuItem("Export...")) {
            open_export_dialog();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close"))  {
            running_ = false;
        }
        ImGui::EndMenu();
    }
}

void rumpu::edit_menu() {
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, undo_.can_undo())) {
            perform_undo();
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z", false, undo_.can_redo())) {
            perform_redo();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Song")) {
        if (ImGui::MenuItem("Properties...")) {
            song_properties_dialog_.open(&song_);
        }
        ImGui::EndMenu();
    }
}

void rumpu::clone_current_section() {
    if (auto* sec = song_.find_section(current_section_)) {
        std::uint32_t id{};
        {
            song_edit edit{song_, &undo_};
            id = song_.add_section(*sec);
            song_.section_order().push_back(id);
        }
        current_section_ = id;
        track_edit_view_->set_context(&song_, id, &undo_);
    }
}

void rumpu::remove_current_section() {
    {
        song_edit edit{song_, &undo_};
        song_.remove_section(current_section_);
    }
    if (!song_.sections().empty()) {
        select_section_impl(song_.sections().begin()->first);
    }
}

void rumpu::sections_menu() {
    if (ImGui::BeginMenu("Sections")) {
        if (ImGui::MenuItem("Clone section")) {
            clone_current_section();
        }
        if (ImGui::MenuItem("Remove section", nullptr, false, song_.sections().size() > 1)) {
            remove_current_section();
        }
        ImGui::Separator();
        for (auto const& [id, section] : song_.sections()) {
            std::string label = "Section " + std::to_string(id);
            bool is_selected = (id == current_section_);
            if (ImGui::MenuItem(label.c_str(), nullptr, is_selected)) {
                select_section_impl(id);
            }
        }
        ImGui::EndMenu();
    }
}

void rumpu::other_menus() {
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
        if (ImGui::MenuItem("Add instruments from folder...")) {
            add_instruments_from_folder_dialog_.open();
        }
        if (ImGui::MenuItem("Manage instruments...")) {
            instruments_dialog_.open(&song_, &undo_, project_dir(current_file_));
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Options")) {
        if (ImGui::MenuItem("Follow play cursor", nullptr, follow_cursor_)) {
            follow_cursor_ = !follow_cursor_;
            track_edit_view_->set_follow_cursor(follow_cursor_);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
        ImGui::Separator();
        if (ImGui::MenuItem("About")) {
            about_dialog_.open();
        }
        ImGui::EndMenu();
    }
}

void rumpu::handle_undo_shortcut() {
    // Only act on the shortcut when no text field is being edited and no modal
    // (e.g. an in-progress export) is open, so undo cannot clobber a text edit
    // or swap the song out from under a dialog that holds pointers into it.
    if (!ImGui::GetIO().WantTextInput
        && !ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup)
        && ImGui::IsKeyDown(ImGuiMod_Ctrl) && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (ImGui::IsKeyDown(ImGuiMod_Shift)) {
            perform_redo();
        } else {
            perform_undo();
        }
    }
}

void rumpu::draw_dialogs() {
    about_dialog_.draw();
    add_instrument_dialog_.draw();
    add_instruments_from_folder_dialog_.draw();
    add_track_dialog_.draw();
    new_song_dialog_.draw();
    song_properties_dialog_.draw();
    instruments_dialog_.draw();
    export_dialog_.draw();
    draw_error_dialog();
}

void rumpu::draw_windows() {
    if (player_.is_playing()) {
        auto status = player_.get_status();
        if (follow_cursor_ && status.section_id != current_section_ && song_.find_section(status.section_id)) {
            select_section_impl(status.section_id);
        }
        track_edit_view_->set_play_status(status);
    }

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
}

bool rumpu::update() {
    std::unique_lock l{mutex_};

    poll_project_dialog_result();

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
    handle_undo_shortcut();
    draw_dialogs();
    draw_windows();

    ImGui::End();
    ImGui::PopStyleVar(1);

    return running_;
}

void rumpu::add_track(uint32_t section, std::size_t instrument_index) {
    std::unique_lock l{mutex_};
    {
        song_edit edit{song_, &undo_};
        std::string name;
        if(instrument_index < song_.instruments().size()) {
            name = song_.instruments()[instrument_index].name();
        }
        song_.add_track(instrument_index, name);
    }
    track_edit_view_->set_context(&song_, section, &undo_);
}

void rumpu::open_add_track_dialog(uint32_t section) {
    add_track_dialog_.open(&song_, section);
}

void rumpu::select_section_impl(uint32_t section_id) {
    current_section_ = section_id;
    track_edit_view_->set_context(&song_, section_id, &undo_);
    player_pos_changed();
}

void rumpu::select_section(uint32_t section_id) {
    std::unique_lock l{mutex_};
    select_section_impl(section_id);
}

void rumpu::add_instrument(std::string path, std::string name) {
    std::unique_lock l{mutex_};
    auto base = project_dir(current_file_);
    instrument inst{project_relative_path(path, base)};
    if (!name.empty()) {
        inst.set_name(std::move(name));
    }
    song_edit edit{song_, &undo_};
    song_.add_instrument(std::move(inst));
}

void rumpu::add_instruments(std::vector<std::string> paths) {
    if (paths.empty()) {
        return;
    }

    std::unique_lock l{mutex_};
    namespace fs = std::filesystem;

    auto base = project_dir(current_file_);
    song_edit edit{song_, &undo_};
    for (auto const& path : paths) {
        fs::path p{path};
        instrument inst{project_relative_path(path, base)};
        inst.set_name(p.stem().string());
        song_.add_instrument(std::move(inst));
    }

    LOG_INFO("added {} instruments", paths.size());
}

void rumpu::add_section() {
    std::unique_lock l{mutex_};
    LOG_TRACE("add_section");
    std::uint32_t id{};
    {
        song_edit edit{song_, &undo_};
        id = song_.add_section();
        song_.section_order().push_back(id);
    }
    current_section_ = id;
    track_edit_view_->set_context(&song_, id, &undo_);
}

void rumpu::load_and_play(uint32_t section) {
    try {
        // Fetch the sample rate before taking the song lock: the audio thread
        // takes the player lock then the song lock, so we must never hold the
        // song lock while calling into the player.
        auto sample_rate = player_.sample_rate();
        {
            song_edit edit{song_};
            song_.load_instruments(sample_rate, project_dir(current_file_));
        }
        player_.play(&song_, false, section);
    } catch(std::exception const& e) {
        LOG_WARN("play failed: {}", e.what());
        show_error(std::format("Failed to play: {}", e.what()));
    }
}

void rumpu::play_section(uint32_t section) {
    std::unique_lock l{mutex_};
    load_and_play(section);
}

void rumpu::play_song() {
    std::unique_lock l{mutex_};
    load_and_play(0);
}

void rumpu::stop_song(uint32_t section) {
    std::unique_lock l{mutex_};
    player_.stop();
}

void rumpu::remove_track(std::size_t index) {
    std::unique_lock l{mutex_};
    {
        song_edit edit{song_, &undo_};
        song_.remove_instrument(index);
    }
    track_edit_view_->set_context(&song_, current_section_, &undo_);
}

void rumpu::open_project(std::string path) {
    std::unique_lock l{mutex_};
    // Stop the player before touching the song lock (see load_and_play) so the
    // audio thread is no longer reading the song when we swap it out.
    player_.stop();
    try {
        song loaded = load_song_file(path);
        // only clear history once the load has succeeded, so a failed open
        // keeps the current song's undo intact
        undo_.clear();
        {
            song_edit edit{song_};
            song_ = std::move(loaded);
        }
        current_file_ = path;
        auto const& order = song_.section_order();
        uint32_t section_id = order.empty() ? 0 : order.front();
        current_section_ = section_id;
        track_edit_view_->set_context(&song_, section_id, &undo_);
    } catch(std::exception const& e) {
        LOG_WARN("load_song_file failed: {}", e.what());
        show_error(std::format("Failed to open project: {}", e.what()));
    }
}

void rumpu::save_project(std::string path) {
    std::unique_lock l{mutex_};
    try {
        save_song_file(path, song_);
        current_file_ = path;
    } catch(std::exception const& e) {
        LOG_WARN("save_song_file failed: {}", e.what());
        show_error(std::format("Failed to save project: {}", e.what()));
    }
}

void rumpu::new_song(std::string name, time_signature ts, float tempo) {
    std::unique_lock l{mutex_};
    player_.stop();
    undo_.clear();
    std::uint32_t id{};
    {
        song_edit edit{song_};
        song_ = song{song_metainfo{std::move(name), "", ""}, ts, drum::tempo{tempo}};
        id = song_.add_section();
        song_.section_order().push_back(id);
    }
    current_file_.clear();
    current_section_ = id;
    track_edit_view_->set_context(&song_, id, &undo_);
}

void rumpu::update_song_properties(std::string name, std::string author, std::string notes, time_signature ts, float tempo, float rand_offset_ms, float rand_volume_percent) {
    std::unique_lock l{mutex_};
    song_edit edit{song_, &undo_};
    song_.set_metainfo(song_metainfo{std::move(name), std::move(author), std::move(notes)});
    song_.set_default_time_signature(ts);
    song_.set_default_tempo(drum::tempo{tempo});
    song_.set_rand_offset(rand_hit_offset{rand_offset_ms});
    song_.set_rand_volume(rand_hit_volume{rand_volume_percent});
}

void rumpu::player_pos_changed() {
    // Only needed for stop: clears the cursor when playback ends
    if (!player_.is_playing()) {
        track_edit_view_->set_play_status(player_.get_status());
    }
}

void rumpu::show_error(std::string message) {
    error_message_ = std::move(message);
    error_pending_ = true;
}

void rumpu::draw_error_dialog() {
    if (error_pending_) {
        ImGui::OpenPopup("Error");
        error_pending_ = false;
    }
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", error_message_.c_str());
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void rumpu::handle_event(std::unique_ptr<securepath::event_system::event_base> ev) {
    dispatch(*ev
        , event_dest<event::add_track>(&rumpu::add_track)
        , event_dest<event::open_add_track_dialog>(&rumpu::open_add_track_dialog)
        , event_dest<event::add_instrument>(&rumpu::add_instrument)
        , event_dest<event::add_instruments>(&rumpu::add_instruments)
        , event_dest<event::play_section>(&rumpu::play_section)
        , event_dest<event::play_song>(&rumpu::play_song)
        , event_dest<event::stop_song>(&rumpu::stop_song)
        , event_dest<event::select_section>(&rumpu::select_section)
        , event_dest<event::add_section>(&rumpu::add_section)
        , event_dest<event::remove_track>(&rumpu::remove_track)
        , event_dest<event::open_project>(&rumpu::open_project)
        , event_dest<event::save_project>(&rumpu::save_project)
        , event_dest<event::new_song>(&rumpu::new_song)
        , event_dest<event::update_song_properties>(&rumpu::update_song_properties)
        , event_dest<drum::event::player_pos_changed>(&rumpu::player_pos_changed)
        );
}


}

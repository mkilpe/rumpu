#pragma once

#include "async_dialog_result.hpp"

#include "imgui.h"

#include <utility>

namespace securepath::drum::app {

// UI entry ranges for song timing values, deliberately narrower than the
// engine limits (tempo::min_bpm/max_bpm and the loader's 1..128 signatures)
inline constexpr int min_ui_signature_value = 1;
inline constexpr int max_ui_signature_value = 32;
inline constexpr float min_ui_tempo = 20.0f;
inline constexpr float max_ui_tempo = 400.0f;

// the time-signature + tempo widget trio shared by the new-song and
// song-properties dialogs
void timing_fields(int& beats, int& beat_type, float& tempo);

// "Browse..."-style button that is disabled while a chooser is already open
// and launches the native dialog under the mailbox's session token
template<typename OpenDialogFn>
void browse_button(char const* label, async_dialog_result_ptr const& result, OpenDialogFn&& open) {
    bool const browsing = result->in_flight();
    if (browsing) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button(label)) {
        launch_dialog(result, std::forward<OpenDialogFn>(open));
    }
    if (browsing) {
        ImGui::EndDisabled();
    }
}

}

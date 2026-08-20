
#include "dialog_widgets.hpp"

#include <algorithm>

namespace securepath::drum::app {

void timing_fields(int& beats, int& beat_type, float& tempo) {
    ImGui::Text("Time signature:");
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Beats##beats", &beats);
    beats = std::clamp(beats, min_ui_signature_value, max_ui_signature_value);
    ImGui::SetNextItemWidth(100);
    ImGui::InputInt("Beat type##type", &beat_type);
    beat_type = std::clamp(beat_type, min_ui_signature_value, max_ui_signature_value);

    ImGui::Text("Tempo (BPM):");
    ImGui::SetNextItemWidth(100);
    ImGui::InputFloat("##tempo", &tempo, 1.0f, 10.0f, "%.1f");
    tempo = std::clamp(tempo, min_ui_tempo, max_ui_tempo);
}

}

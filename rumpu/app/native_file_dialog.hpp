#pragma once
#include <functional>
#include <string>

namespace securepath::drum::app {

// Opens a native WAV file selection dialog asynchronously.
// callback is called on a background thread with the selected path,
// or an empty string if the user cancelled.
void open_wav_file_dialog(std::function<void(std::string)> callback);
void save_wav_file_dialog(std::function<void(std::string)> callback);

void open_project_file_dialog(std::function<void(std::string)> callback);
void save_project_file_dialog(std::function<void(std::string)> callback);

}

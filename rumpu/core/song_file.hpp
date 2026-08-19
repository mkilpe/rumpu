#pragma once

#include "song.hpp"

#include <filesystem>
#include <string>

namespace securepath::drum {

song load_song_file(std::string const& file);
void save_song_file(std::string const& file, song const&);

// Path to store in the project for a user-picked file: relative to the
// project directory when possible, the original path when not (no project
// directory yet, different drive, or relative computation failure).
std::string project_relative_path(std::string const& path, std::filesystem::path const& base);

// Rejects decoded songs whose values would crash or hang the app (bad
// instrument indices, zero tempo/length, mismatched layouts...). Called by
// load_song_file; throws std::runtime_error with a descriptive message.
void validate_song(song const&);

}

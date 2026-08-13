#pragma once

#include "song.hpp"

namespace securepath::drum {

song load_song_file(std::string const& file);
void save_song_file(std::string const& file, song const&);

// Rejects decoded songs whose values would crash or hang the app (bad
// instrument indices, zero tempo/length, mismatched layouts...). Called by
// load_song_file; throws std::runtime_error with a descriptive message.
void validate_song(song const&);

}

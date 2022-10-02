#ifndef SPDRUM_COMMON_SONG_FILE_HEADER
#define SPDRUM_COMMON_SONG_FILE_HEADER

#include "song.hpp"

namespace securepath::drum {

song load_song_file(std::string const& file);
void save_song_file(std::string const& file, song const&);

}

#endif
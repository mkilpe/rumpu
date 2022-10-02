#ifndef SPDRUM_COMMON_EXPORT_HEADER
#define SPDRUM_COMMON_EXPORT_HEADER

#include "song.hpp"

#include <securepath/audio/audio_lib/audio_format.hpp>
#include <securepath/util/octet_vector.hpp>

#include <cstdint>
#include <string>

namespace securepath::drum {

struct export_options {
	// later on perhaps support soft clipping and such
	enum gain_control_type { none, peak_normalise, user_gain } gain_control{peak_normalise};
	float user_gain_value{0.0f};
};

void export_as_wav(std::string const& file, song const&, export_options = {});

}

#endif
#pragma once

#include "song.hpp"
#include "mixer.hpp"

#include <securepath/audio/audio_lib/audio_format.hpp>

#include <deque>
#include <string>

namespace securepath::drum {

struct export_options {
	// later on perhaps support soft clipping and such
	enum gain_control_type { none, peak_normalise, user_gain } gain_control{peak_normalise};
	float user_gain_value{0.0f};
	audio::audio_format format{audio::short_t, 1, 16, 44100, std::endian::little};
};

class wav_exporter {
public:
	wav_exporter(std::string file, song const& s, export_options opts = {});

	// Returns true if more work remains, false when export is complete.
	bool process();

	// Seconds of audio processed so far.
	float progress() const { return mix_.play_position(); }

	// Total duration of the song in seconds.
	float duration() const { return mix_.duration(); }

private:
	std::string file_;
	export_options opts_;
	mixer mix_;
	std::deque<float> song_data_;

	bool mixing_done_{false};
};

}

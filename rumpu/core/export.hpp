#pragma once

#include "song.hpp"
#include "mixer.hpp"

#include <securepath/audio/audio_lib/audio_format.hpp>

#include <deque>
#include <filesystem>
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
	// Takes ownership of the song snapshot. The samples are loaded and
	// resampled to the export rate incrementally by process(), one sample per
	// call, so a caller can keep drawing progress while the disk work runs
	// (relative sample paths resolve against base_dir).
	wav_exporter(std::string file, song s, export_options opts = {}, std::filesystem::path base_dir = {});

	// The mixer keeps a pointer to the owned song, so a copied or moved
	// exporter would leave it dangling.
	wav_exporter(wav_exporter const&) = delete;
	wav_exporter& operator=(wav_exporter const&) = delete;

	// Returns true if more work remains, false when export is complete.
	bool process();

	// True while process() is still loading and resampling samples.
	bool is_loading() const { return samples_loaded_ < samples_total_; }

	// Sample loading progress counts.
	std::size_t samples_loaded() const { return samples_loaded_; }
	std::size_t samples_total() const { return samples_total_; }

	// Seconds of audio processed so far.
	float progress() const { return mix_.play_position(); }

	// Total duration of the song in seconds.
	float duration() const { return mix_.duration(); }

private:
	// loads the next unloaded sample; false when all samples are loaded
	bool load_next_sample();

	std::string file_;
	export_options opts_;
	std::filesystem::path base_dir_;
	song song_;
	mixer mix_;
	std::deque<float> song_data_;

	std::size_t instrument_index_{};
	std::size_t sample_index_{};
	std::size_t samples_loaded_{};
	std::size_t samples_total_{};
	bool mixing_done_{false};
};

}

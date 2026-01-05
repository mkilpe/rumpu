#pragma once

#include "volume.hpp"
#include "types.hpp"

namespace securepath::drum {

/// information about a single beat
struct beat_hit_data {
	// actual volume of the hit
	drum::volume volume;
	// accent for this beat if any
	volume_accent accent;

	// randomised hit offset in milliseconds
	fp_type rand_hit_offset{};
	// randomise volume
	delta_volume rand_volume;
};

// dictate how the beat ends, fall off
struct beat_stop_data {
	audio_falloff falloff;
};

struct beat {
	// each beat is either none (no sound), hit (let the sound continue), stop (ending hit, falloff the sound)
	enum action_type { none, hit, stop } action{none};
	// tells if this beat has been subdivided to multiple beats
	std::vector<beat> division;
	// hit data if this is leaf node with a hit
	beat_hit_data hit_data;
	// stop data if this is leaf node with a stop
	beat_stop_data stop_data;

	float combined_hit_volume() const {
		float vol = hit_data.volume.value;
		vol += hit_data.accent.value;
		vol += hit_data.rand_volume.value;
		return vol;
	}

	beat data() const {
		return beat{action, {}, hit_data, stop_data};
	}
};

struct bar {
	std::vector<beat> beats;
};

}

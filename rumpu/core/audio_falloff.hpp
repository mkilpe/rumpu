#pragma once

#include "types.hpp"

#include <memory>

namespace securepath::drum {

/// runtime falloff used during playback — produced from an audio_falloff definition
class audio_falloff_player {
public:
	virtual ~audio_falloff_player() = default;
	virtual bool is_done() const = 0;
	virtual fp_type factor(fp_type delta) = 0;
};

/// falloff definition stored in beat_stop_data and serialised
class audio_falloff {
public:
	enum falloff_type { immediate = 0, linear = 1, exponential = 2 };

	virtual ~audio_falloff() = default;

	virtual falloff_type type() const = 0;
	virtual fp_type duration_beats() const = 0;
	virtual void set_duration_beats(fp_type) = 0;
	virtual std::unique_ptr<audio_falloff> clone() const = 0;
	/// create a playback instance given current beat duration in seconds
	virtual std::unique_ptr<audio_falloff_player> create_player(fp_type beat_duration_seconds) const = 0;

	/// factory to create a definition by type
	static std::unique_ptr<audio_falloff> create(falloff_type type, fp_type duration_beats = 1.0_fp);
};

}

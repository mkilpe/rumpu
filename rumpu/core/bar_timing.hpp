#pragma once

#include "section.hpp"
#include "tempo.hpp"
#include "time_signature.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>

namespace securepath::drum {

// The per-bar tempo/timing walk shared by the mixer (playback and duration)
// and the player cursor, so slide semantics exist in exactly one place.
//
// Call new_section() when entering a section and begin_bar() for every bar in
// order. Slides accumulate on every bar of their [begin, end) range whether or
// not the bar has a change record; an explicit tempo change on a bar overrides
// the slide for that bar.
struct bar_timing {
	bar_timing() = default;
	bar_timing(time_signature default_ts, tempo start_tempo, std::optional<delta_tempo> global_slide)
	: default_timing(default_ts)
	, current_timing(default_ts)
	, current_tempo(start_tempo)
	, global_tempo_slide(global_slide)
	{
	}

	void new_section() {
		current_tempo_slide = tempo_slide{};
	}

	void begin_bar(section const& sec, std::uint32_t bar) {
		auto change = sec.find_change(bar);
		if(change) {
			if(change->timing_change) {
				current_timing = *change->timing_change;
			}
			if(change->tempo_slide_change) {
				current_tempo_slide = *change->tempo_slide_change;
			}
			if(change->tempo_change) {
				current_tempo = *change->tempo_change;
			}
		}
		bool const explicit_tempo = change && change->tempo_change;
		if(!explicit_tempo) {
			if(current_tempo_slide.is_active(bar)) {
				current_tempo.value += current_tempo_slide.bar_delta();
			}
			if(global_tempo_slide) {
				current_tempo.value += global_tempo_slide->value;
			}
			current_tempo.value = std::clamp(current_tempo.value, tempo::min_bpm, tempo::max_bpm);
		}
	}

	// samples in the current bar at the given sample rate
	std::uint32_t bar_samples(std::uint32_t sample_rate) const {
		double timing_multiplier = default_timing.beat_type() * current_timing.beats_in_bar()
			/ double(default_timing.beats_in_bar() * current_timing.beat_type());
		double const samples =
			60.0 * timing_multiplier * sample_rate * current_timing.beats_in_bar() / current_tempo.value;
		// individually valid but extreme timing/tempo combinations can fall
		// outside uint32: casting such values is undefined, and a zero-sample
		// bar would stall the mixer's render loop
		constexpr double max = std::numeric_limits<std::uint32_t>::max();
		if(!(samples >= 1.0)) {
			return 1;
		}
		if(samples >= max) {
			return std::numeric_limits<std::uint32_t>::max();
		}
		return static_cast<std::uint32_t>(samples);
	}

	time_signature default_timing;
	time_signature current_timing;
	tempo current_tempo;
	std::optional<delta_tempo> global_tempo_slide;
	tempo_slide current_tempo_slide{};
};

}

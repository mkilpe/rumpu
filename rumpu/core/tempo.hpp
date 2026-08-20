#pragma once

#include <cstdint>

namespace securepath::drum {

struct tempo {
	// engine-representable BPM range: the per-bar timing walk clamps
	// slide-driven tempo to it and the project loader rejects values outside it
	static constexpr float min_bpm = 1.0f;
	static constexpr float max_bpm = 9999.0f;

	tempo(float v = 0.0f)
	: value(v)
	{}

	// beats per minute
	float value{};
};

struct delta_tempo {
	float value{};
};

struct tempo_slide {
	std::uint32_t begin{};
	std::uint32_t end{};
	float value;

	bool is_valid() const { return begin != end; }
	bool is_active(std::uint32_t bar) const { return begin <= bar && bar < end; }
	float bar_delta() const { return value / (end-begin); }
};

}

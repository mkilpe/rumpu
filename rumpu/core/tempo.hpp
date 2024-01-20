#pragma once

#include <cstdint>

namespace securepath::drum {

struct tempo {
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

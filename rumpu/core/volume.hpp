#pragma once

#include "types.hpp"

namespace securepath::drum {

/// normalised volume between 0.0 and 1.0
struct volume {
	// separate mute to remember the old value
	bool mute{};
	fp_type value{1.0_fp};

	friend bool operator==(volume const& l, volume const& r) = default;
};


/// amount of volume change
struct delta_volume {
	fp_type value{};

	friend bool operator==(delta_volume const& l, delta_volume const& r) = default;
};


/// sliding volume change
struct volume_slide {
	// begin bar where this slide starts
	std::uint32_t begin{};
	// one beyond bar where this slide ends
	std::uint32_t end{};
	fp_type value{};

	bool is_valid() const { return begin != end; }
	bool is_active(std::uint32_t bar) const { return begin <= bar && bar < end; }
	fp_type bar_delta() const { return value / (end-begin); }
};

/// volume accent
struct volume_accent {
	fp_type value{};

	friend bool operator==(volume_accent const& l, volume_accent const& r) = default;
};

/// volume accent settings (strength, accent pattern, etc)
struct volume_accent_info {
};

}

#ifndef SPDRUM_COMMON_BAR_HEADER
#define SPDRUM_COMMON_BAR_HEADER

#include "volume.hpp"

#include <securepath/serialisation/enum.hpp>
#include <securepath/serialisation/vector.hpp>
#include <securepath/serialisation/sequence.hpp>

#include <cstdint>
#include <optional>
#include <vector>

namespace securepath::drum {

struct hit_data {
	drum::volume volume;
	volume_accent accent;
	// randomised hit offset in milliseconds
	float rand_hit_offset{};
	delta_volume rand_volume;

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & volume & accent & rand_hit_offset & rand_volume;
	}
};

struct stop_data {
	audio_falloff falloff;

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & falloff;
	}
};

struct beat {
	enum action_type { none, hit, stop } action{none};
	// tells if this beat has been subdivided to multiple beats
	std::vector<beat> division;
	// hit data if this is leaf node with a hit
	drum::hit_data hit_data;
	// stop data if this is leaf node with a stop
	drum::stop_data stop_data;

	float combined_hit_volume() const {
		float vol = hit_data.volume.value;
		vol += hit_data.accent.value;
		vol += hit_data.rand_volume.value;
		return vol;
	}

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & action & division & hit_data & stop_data;
	}
};

struct bar {
	std::vector<beat> beats;

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & beats;
	}
};

// qt meta-system requires comparison

inline bool operator==(hit_data const& l, hit_data const& r) {
	return l.accent == r.accent	&& l.rand_hit_offset == r.rand_hit_offset
		&& l.rand_volume == r.rand_volume;
}

inline bool operator!=(hit_data const& l, hit_data const& r) {
	return !(l == r);
}

inline bool operator==(stop_data const& l, stop_data const& r) {
	return l.falloff == r.falloff;
}

inline bool operator!=(stop_data const& l, stop_data const& r) {
	return !(l == r);
}

inline bool operator==(beat const& l, beat const& r) {
	return l.action == r.action && l.division == r.division
		&& l.hit_data == r.hit_data && l.stop_data == r.stop_data;
}

inline bool operator!=(beat const& l, beat const& r) {
	return !(l == r);
}

inline bool operator==(bar const& l, bar const& r) {
	return l.beats == r.beats;
}

inline bool operator!=(bar const& l, bar const& r) {
	return !(l == r);
}

}

#endif
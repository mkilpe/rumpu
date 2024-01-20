#ifndef SPDRUM_COMMON_VOLUME_HEADER
#define SPDRUM_COMMON_VOLUME_HEADER

#include <securepath/serialisation/sequence.hpp>
#include <securepath/serialisation/tag.hpp>

#include <cstdint>
#include <cmath>
#include <optional>

namespace securepath::drum {

/// normalised volume between 0.0 and 1.0
struct volume {
	// separate mute to remember the old value
	bool mute{};
	float value{1.0f};

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & mute & value;
	}

	friend bool operator==(volume const& l, volume const& r) = default;
};


/// amount of volume change
struct delta_volume {
	float value{};

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & value;
	}

	friend bool operator==(delta_volume const& l, delta_volume const& r) = default;
};


/// sliding volume change
struct volume_slide {
	// begin bar where this slide starts
	std::uint32_t begin{};
	// one beyond bar where this slide ends
	std::uint32_t end{};
	float value{};

	bool is_valid() const { return begin != end; }
	bool is_active(std::uint32_t bar) const { return begin <= bar && bar < end; }
	float bar_delta() const { return value / (end-begin); }

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & begin & end & value;
	}
};

/// volume accent
struct volume_accent {
	float value{};

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & value;
	}

	friend bool operator==(volume_accent const& l, volume_accent const& r) = default;
};


/// volume accent settings (strength, accent pattern, etc)
struct volume_accent_info {
	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
	}
};

struct audio_falloff {
	enum falloff_type { immediate, linear, exponential } type{ exponential };
	float pos{};
	float end{};

	bool is_done() const {
		return pos >= end;
	}

	float factor(float delta) {
		float ret = 0.0;
		pos += delta;
		if(pos > end) {
			pos = end;
		}
		if(type == linear) {
			ret = 1.0f - pos/end;
		} else if(type == exponential) {
			float x = pos/end*7-5;
			ret = 1.0f - std::exp2(x) / 4;
		}
		return ret;
	}

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & type & pos & end;
	}

	friend bool operator==(audio_falloff const& l, audio_falloff const& r) = default;
};


}

#endif
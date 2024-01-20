#ifndef SPDRUM_COMMON_TEMPO_HEADER
#define SPDRUM_COMMON_TEMPO_HEADER

#include <securepath/serialisation/sequence.hpp>

#include <cstdint>

namespace securepath::drum {

struct tempo {
	tempo(float v = 0.0f)
	: value(v)
	{}

	// beats per minute
	float value{};

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & value;
	}
};

struct delta_tempo {
	float value{};

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & value;
	}
};

struct tempo_slide {
	std::uint32_t begin{};
	std::uint32_t end{};
	float value;

	bool is_valid() const { return begin != end; }
	bool is_active(std::uint32_t bar) const { return begin <= bar && bar < end; }
	float bar_delta() const { return value / (end-begin); }

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & begin & end & value;
	}
};

}

#endif
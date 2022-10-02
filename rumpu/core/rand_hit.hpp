#ifndef SPDRUM_COMMON_RAND_HIT_HEADER
#define SPDRUM_COMMON_RAND_HIT_HEADER

#include <securepath/serialisation/sequence.hpp>

namespace securepath::drum {

// settings for randomising hit volume
struct rand_hit_volume {
	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
	}
};

// settings for randomising hit offset
struct rand_hit_offset {
	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
	}
};

}

#endif
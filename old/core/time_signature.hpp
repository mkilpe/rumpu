#ifndef SPDRUM_COMMON_TIME_SIGNATURE_HEADER
#define SPDRUM_COMMON_TIME_SIGNATURE_HEADER

#include <securepath/serialisation/sequence.hpp>

#include <cstdint>

namespace securepath::drum {

class time_signature {
public:
	time_signature() = default;
	time_signature(std::uint16_t beats, std::uint16_t type)
	: beats_in_bar_(beats)
	, beat_type_(type)
	{}

	bool is_valid() const { return beats_in_bar_ != 0 && beat_type_ != 0; }

	/// returns the upper part of the time signature (3 in the 3/4)
	std::uint16_t beats_in_bar() const { return beats_in_bar_; }

	/// returns the lower part of the time signature (4 in the 3/4)
	std::uint16_t beat_type() const { return beat_type_; }

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & beats_in_bar_ & beat_type_;
	}
private:
	std::uint16_t beats_in_bar_{};
	std::uint16_t beat_type_{};
};

}

#endif
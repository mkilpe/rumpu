#ifndef SPDRUM_COMMON_TRACK_HEADER
#define SPDRUM_COMMON_TRACK_HEADER

#include "bar.hpp"
#include "instrument.hpp"

#include <securepath/serialisation/map.hpp>
#include <securepath/serialisation/sequence.hpp>

#include <deque>

namespace securepath::drum {

class track {
public:
	track() = default;
	explicit track(std::uint32_t length)
	: bars_(length)
	{
	}

	std::deque<bar>& bars() { return bars_; }
	std::deque<bar> const& bars() const { return bars_; }

	std::optional<volume_slide> find_volume_slide(std::uint32_t index) const {
		auto it = volume_slides_.find(index);
		return it != volume_slides_.end() ? it->second : std::optional<volume_slide>();
	}

	void set_length(std::uint32_t l) {
		bars_.resize(l);
	}

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & bars_ & volume_slides_;
	}
private:
	std::deque<bar> bars_;
	std::map<std::uint32_t, volume_slide> volume_slides_;
};

}

#endif
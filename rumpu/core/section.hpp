#pragma once

#include "bar.hpp"
#include "tempo.hpp"
#include "time_signature.hpp"
#include "track.hpp"

#include <securepath/serialisation/deque.hpp>
#include <securepath/serialisation/map.hpp>
#include <securepath/serialisation/sequence.hpp>

#include <cstdint>
#include <optional>
#include <map>

namespace securepath::drum {

struct section_bar_change {
	std::optional<time_signature> timing_change;
	std::optional<tempo> tempo_change;
	std::optional<tempo_slide> tempo_slide_change;
};

class section {
public:
	section(std::uint32_t length = 0, std::uint32_t tracks = 0)
	: length_(length)
	, tracks_(tracks, track{length_})
	{
	}

	std::uint32_t length() const { return length_; }
	std::deque<track>& tracks() { return tracks_; }
	std::deque<track> const& tracks() const { return tracks_; }

	track& add_track() {
		tracks_.push_back(track{length_});
		return tracks_.back();
	}

	std::optional<section_bar_change> find_change(std::uint32_t index) const {
		auto it = changes_.find(index);
		return it != changes_.end() ? it->second : std::optional<section_bar_change>();
	}

	void set_length(std::uint32_t l) {
		length_ = l;
		for(auto&& v : tracks_) {
			v.set_length(l);
		}
	}

private:
	// length of this section in bars
	std::uint32_t length_{};
	// the actual bar data for the tracks
	std::deque<track> tracks_;
	// section wide changes with bar position
	std::map<std::uint32_t, section_bar_change> changes_;
};

}

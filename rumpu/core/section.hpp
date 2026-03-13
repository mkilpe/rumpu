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
#include <string>

namespace securepath::drum {

struct section_bar_change {
	std::optional<time_signature> timing_change;
	std::optional<tempo> tempo_change;
	std::optional<tempo_slide> tempo_slide_change;
};

class section {
public:
	section(std::uint32_t length = 0, std::uint32_t num_tracks = 0)
	: length_(length)
	{
		for (std::uint32_t i = 0; i < num_tracks; ++i)
			tracks_.emplace_back(i, length_);
	}

	std::string const& name() const { return name_; }
	void set_name(std::string n) { name_ = std::move(n); }

	std::uint32_t length() const { return length_; }
	auto& tracks(this auto& self) { return self.tracks_; }

	track& add_track(std::size_t instrument_index) {
		tracks_.emplace_back(instrument_index, length_);
		return tracks_.back();
	}

	std::optional<section_bar_change> find_change(std::uint32_t index) const {
		auto it = changes_.find(index);
		return it != changes_.end() ? it->second : std::optional<section_bar_change>();
	}

	void set_tempo_change(std::uint32_t bar_index, std::optional<tempo> t) {
		auto& change = changes_[bar_index];
		change.tempo_change = t;
		if (!change.timing_change && !change.tempo_change && !change.tempo_slide_change)
			changes_.erase(bar_index);
	}

	void set_length(std::uint32_t l) {
		length_ = l;
		for(auto&& v : tracks_) {
			v.set_length(l);
		}
	}

	template<typename Ar>
	friend Ar& serialise(Ar&, section&);

private:
	std::string name_;
	// length of this section in bars
	std::uint32_t length_{};
	// the actual bar data for the tracks
	std::deque<track> tracks_;
	// section wide changes with bar position
	std::map<std::uint32_t, section_bar_change> changes_;
};

}

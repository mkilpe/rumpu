#pragma once

#include "bar.hpp"
#include "instrument.hpp"

#include <securepath/serialisation/map.hpp>
#include <securepath/serialisation/sequence.hpp>

#include <cstdint>
#include <deque>
#include <random>

namespace securepath::drum {

class track {
public:
	track() = default;
	explicit track(std::uint32_t length)
	: bars_(length)
	{
	}
	track(std::size_t instrument_index, std::uint32_t length)
	: instrument_index_(instrument_index)
	, bars_(length)
	{
	}

	auto& bars(this auto& self) { return self.bars_; }

	std::string const& name() const { return name_; }
	void set_name(std::string n) { name_ = std::move(n); }

	std::size_t instrument_index() const { return instrument_index_; }
	void set_instrument_index(std::size_t i) { instrument_index_ = i; }

	drum::volume const& volume() const { return volume_; }
	void set_volume(drum::volume const& v) { volume_ = v; }

	std::uint32_t random_seed() const { return random_seed_; }
	void set_random_seed(std::uint32_t s) { random_seed_ = s; }

	std::optional<volume_slide> find_volume_slide(std::uint32_t index) const {
		auto it = volume_slides_.find(index);
		return it != volume_slides_.end() ? it->second : std::optional<volume_slide>();
	}

	void set_length(std::uint32_t l) {
		bars_.resize(l);
	}
	template<typename Ar>
	friend Ar& serialise(Ar&, track&);

private:
	std::string name_;
	std::size_t instrument_index_{};
	std::deque<bar> bars_;
	std::map<std::uint32_t, volume_slide> volume_slides_;
	drum::volume volume_;
	std::uint32_t random_seed_{std::random_device{}()};
};

}

#pragma once

#include "drum_sample.hpp"
#include "volume.hpp"

namespace securepath::drum {

class instrument {
public:
	instrument() = default;
	instrument(std::string const& file);

	bool is_valid() const;
	std::string const& name() const;
	drum_sample const& sample_to_play() const;
	drum::volume const& volume() const;

	void set_volume(drum::volume const&);
	void load_samples(std::uint32_t sample_rate);

	template<typename Ar>
	friend Ar& serialise(Ar&, instrument&);

private:
	std::string name_;
	std::vector<drum_sample> samples_;
	drum::volume volume_;
};

}

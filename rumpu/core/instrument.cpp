#include "instrument.hpp"

namespace securepath::drum {

instrument::instrument(std::string const& file)
: name_(file)
{
	samples_.push_back(load_drum_sample(file));
}

bool instrument::is_valid() const {
	return !samples_.empty();
}

drum_sample const& instrument::sample_to_play() const {
	return samples_.front();
	//t: later on allow to select randomly from multiple samples
}

std::string const& instrument::name() const {
	return name_;
}

drum::volume const& instrument::volume() const {
	return volume_;
}

void instrument::set_volume(drum::volume const& v) {
	volume_ = v;
}

void instrument::load_samples(std::uint32_t sample_rate) {
	for(auto& v : samples_) {
		v.load_sample(sample_rate);
	}
}

}
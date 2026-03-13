#include "instrument.hpp"

namespace securepath::drum {

instrument::instrument(std::string const& file)
: name_(file)
{
	samples_.emplace_back(file);
}

bool instrument::is_valid() const {
	return !samples_.empty();
}

bool instrument::is_loaded() const {
	for(auto const& s : samples_) {
		if(!s.buffer()) return false;
	}
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

void instrument::set_name(std::string name) {
	name_ = std::move(name);
}

void instrument::set_volume(drum::volume const& v) {
	volume_ = v;
}

void instrument::load_samples(std::uint32_t sample_rate, std::filesystem::path const& base_dir) {
	for(auto&& s : samples_) {
		s.load_sample(sample_rate, base_dir);
	}
}

}
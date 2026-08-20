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
	// Returns a representative sample for UI/label use. The mixer picks a
	// random sample per hit directly via samples() + its per-track RNG.
	return samples_.front();
}

std::vector<drum_sample> const& instrument::samples() const {
	return samples_;
}

std::size_t instrument::sample_count() const {
	return samples_.size();
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

void instrument::add_sample(std::string file) {
	samples_.emplace_back(std::move(file));
}

void instrument::remove_sample(std::size_t index) {
	if (index < samples_.size()) {
		samples_.erase(samples_.begin() + index);
	}
}

void instrument::load_samples(std::uint32_t sample_rate, std::filesystem::path const& base_dir) {
	for(auto&& s : samples_) {
		s.load_sample(sample_rate, base_dir);
	}
}

void instrument::load_sample(std::size_t index, std::uint32_t sample_rate, std::filesystem::path const& base_dir) {
	if(index < samples_.size()) {
		samples_[index].load_sample(sample_rate, base_dir);
	}
}

}
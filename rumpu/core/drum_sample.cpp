
#include "drum_sample.hpp"

#include <stdexcept>

namespace securepath::drum {

drum_sample::drum_sample(std::string file)
: source_file_(std::move(file))
{
}

bool drum_sample::load_sample(std::uint32_t sample_rate) {
	return false;
}

drum_sample load_drum_sample(std::string const& file, std::uint32_t sample_rate) {
	drum_sample sample(file);
//	sample.load_sample(sample_rate);
	return sample;
}

}

#pragma once

#include "types.hpp"

#include <memory>
#include <string>

namespace securepath::drum {

using sample_buffer = std::shared_ptr<std::vector<float> const>;

class drum_sample {
public:
	drum_sample() = default;
	drum_sample(std::string file);

	std::string source_file() const { return source_file_; }
	sample_buffer buffer() const { return buffer_; }
	std::uint32_t sample_rate() const { return sample_rate_; }

	bool load_sample(std::uint32_t sample_rate);

private:
	std::string source_file_;

	//these two are not serialised but rather filled when loaded
	std::uint32_t sample_rate_{};
	sample_buffer buffer_;
};

drum_sample load_drum_sample(std::string const& file, std::uint32_t sample_rate = 44100);

}
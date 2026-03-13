
#include "drum_sample.hpp"

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/log/log.hpp>

#include <cstring>
#include <stdexcept>

namespace securepath::drum {

drum_sample::drum_sample(std::string file)
: source_file_(std::move(file))
{
}

void drum_sample::load_sample(std::uint32_t sample_rate, std::filesystem::path const& base_dir) {
	if(source_file_.empty()) {
		throw std::runtime_error("Failed to load sample");
	}
	std::filesystem::path p{source_file_};
	std::string resolved = (p.is_relative() && !base_dir.empty()) ? (base_dir / p).string() : source_file_;
	audio::audio_format target{audio::float_t, 1, 32, sample_rate};
	audio::audio_data pcm;
	pcm.load(resolved, target);

	auto const& raw = pcm.data();
	auto buf = std::make_shared<std::vector<float>>(raw.size() / sizeof(float));
	std::memcpy(buf->data(), raw.data(), raw.size());

	buffer_ = std::move(buf);
	sample_rate_ = sample_rate;
}

drum_sample load_drum_sample(std::string const& file, std::uint32_t sample_rate) {
	drum_sample sample(file);
	sample.load_sample(sample_rate);
	return sample;
}

}

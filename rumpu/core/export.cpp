#include "export.hpp"

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/log/log.hpp>

#include <cstring>
#include <deque>

namespace securepath::drum {

static void peak_normalise(std::deque<float>& song_data) {
	float min{}, max{};
	for(auto v : song_data) {
		min = std::min(min, v);
		max = std::max(max, v);
	}
	float div = std::max(std::abs(min), std::abs(max));
	LOG_TRACE("peak normalising with value {}", div);
	for(auto& v : song_data) {
		v /= div;
	}
}

constexpr std::size_t chunk_size = 50000;

wav_exporter::wav_exporter(std::string file, song const& s, export_options opts)
	: file_(std::move(file))
	, opts_(opts)
	, mix_(s, opts_.format.samples_per_second)
{}

bool wav_exporter::process() {
	if(!mixing_done_) {
		float arr[chunk_size];
		std::size_t const size = mix_.process(arr, chunk_size);
		if(size) {
			song_data_.insert(song_data_.end(), arr, arr + size);
			return true;
		}
		mixing_done_ = true;
	}

	if(opts_.gain_control == export_options::peak_normalise) {
		peak_normalise(song_data_);
	}

	octet_vector float_data(song_data_.size() * sizeof(float));
	auto* dst = float_data.data();
	for(float v : song_data_) {
		std::memcpy(dst, &v, sizeof(float));
		dst += sizeof(float);
	}

	audio::audio_format const float_fmt{audio::float_t, 1, 32, opts_.format.samples_per_second, std::endian::native};
	audio::audio_data ad(float_fmt, std::move(float_data));
	ad.save(file_, opts_.format);
	return false;
}

}

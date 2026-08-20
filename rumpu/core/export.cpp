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
	// a silent song has nothing to normalise (and would divide by zero)
	if(div > 0.0f) {
		for(auto& v : song_data) {
			v /= div;
		}
	}
}

constexpr std::size_t chunk_size = 50000;

wav_exporter::wav_exporter(std::string file, song s, export_options opts, std::filesystem::path base_dir)
	: file_(std::move(file))
	, opts_(opts)
	, base_dir_(std::move(base_dir))
	, song_(std::move(s))
	, mix_(song_, opts_.format.samples_per_second)
{
	for(auto const& i : song_.instruments()) {
		samples_total_ += i.sample_count();
	}
}

bool wav_exporter::load_next_sample() {
	auto& instruments = song_.instruments();
	while(instrument_index_ < instruments.size()
		&& sample_index_ >= instruments[instrument_index_].sample_count()) {
		++instrument_index_;
		sample_index_ = 0;
	}
	bool const more = instrument_index_ < instruments.size();
	if(more) {
		instruments[instrument_index_].load_sample(sample_index_, opts_.format.samples_per_second, base_dir_);
		++sample_index_;
		++samples_loaded_;
	}
	return more;
}

bool wav_exporter::process() {
	if(load_next_sample()) {
		return true;
	}

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

#include "export.hpp"
#include "mixer.hpp"

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/log/log.hpp>
#include <securepath/util/byte_order.hpp>
#include <securepath/util/timer.hpp>

#include <deque>

namespace securepath::drum {

static void export_as_wav(std::string const& file, audio::audio_format const& format, octet_vector const& data) {
	audio::audio_data ad(format, std::move(data));
	ad.save(file, audio::audio_data::wav);
}

static octet_vector resample(std::deque<float> const& song_data) {
	octet_vector data(song_data.size()*2);
	for(std::size_t i = 0; i != song_data.size(); ++i) {
		int64_t evalue = song_data[i]*32767;
		int16_t value = static_cast<int16_t>(std::clamp<int64_t>(evalue, -32768, 32767));
		securepath::to_endian<std::int16_t, endian::little>(&data[i*2], value);
	}
	return data;
}

static void peak_normalise(std::deque<float>& song_data) {
	float min{}, max{};
	for(auto v : song_data) {
		min = std::min(min, v);
		max = std::max(max, v);
	}
	float div = std::max(std::abs(min), std::abs(max));
	LOG_TRACE("peak normalising with value %", div);
	for(auto& v : song_data) {
		v /= div;
	}
}

void export_as_wav(std::string const& file, song const& s, export_options ops) {
	timer t;

	audio::audio_format af{audio::short_t, 1, 16, 44100, audio::little_endian};

	std::deque<float> song_data;
	mixer mix(s, false, af.samples_per_second);
	std::size_t size{};
	float arr[50000];
	while((size = mix.process(arr, 50000))) {
		song_data.insert(song_data.end(), arr, arr+size);
	}
	if(ops.gain_control == export_options::peak_normalise) {
		peak_normalise(song_data);
	}
	export_as_wav(file, af, resample(song_data));

	LOG_TRACE("exporting song as wav took %ms", t.elapsed_milliseconds());
}

}

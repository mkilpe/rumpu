// ALSA backend tests. Use the null PCM plugin via ALSA_CONFIG_PATH so no
// audio hardware is needed; skipped when no usable ALSA is present.
#include <catch2/catch_all.hpp>

#include <securepath/audio/audio_lib/audio_buffer.hpp>
#include <securepath/audio/audio_lib/audio_device.hpp>
#include <securepath/audio/audio_lib/audio_interface.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

using namespace securepath;

namespace {

// must take effect before the first ALSA call in the process
struct null_alsa_config {
	null_alsa_config() {
		auto conf = std::filesystem::temp_directory_path() / "rumpu_test_asound.conf";
		std::ofstream(conf.string()) << "pcm.!default {\n\ttype null\n}\n";
		::setenv("ALSA_CONFIG_PATH", conf.string().c_str(), 1);
	}
};

audio::audio_play_device_ptr open_null_device(audio::device_config const& conf) {
	static null_alsa_config config_guard;
	auto iface = audio::create_default_audio_interface();
	audio::audio_play_device_ptr dev;
	if(iface) {
		try {
			dev = iface->play_device(conf);
		} catch(std::exception const&) {
			// no usable ALSA in this environment
		}
	}
	return dev;
}

void check_full_buffer_write(std::uint32_t channels) {
	audio::device_config conf{{audio::float_t, channels, 32, 44100}, 8820};
	auto dev = open_null_device(conf);
	if(!dev) {
		WARN("no ALSA null device available, skipping");
		return;
	}
	auto real = dev->config();
	REQUIRE(real.format.channels == channels);
	// buffer/period sizes are reported in samples, so whole frames only
	CHECK(real.buffer_size % channels == 0);
	CHECK(real.period_size % channels == 0);

	audio::audio_buffer buf(real.format, real.buffer_size);
	auto* p = buf.begin<float>();
	for(std::size_t i = 0; i != real.buffer_size; ++i) {
		p[i] = 0.0f;
	}
	buf.set_used_samples(static_cast<unsigned>(real.buffer_size));

	dev->start();
	auto written = dev->write(buf);
	dev->stop();

	// write() reports samples; a frames/samples mix-up halves or doubles this
	CHECK(written == real.buffer_size);
	CHECK(buf.used_samples() == 0);
}

}

TEST_CASE("alsa stereo write consumes whole buffers", "[alsa_null]") {
	check_full_buffer_write(2);
}

TEST_CASE("alsa mono write consumes whole buffers", "[alsa_null]") {
	check_full_buffer_write(1);
}

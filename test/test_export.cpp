#include <catch2/catch_all.hpp>

#include <rumpu/core/export.hpp>
#include <rumpu/core/song.hpp>
#include <rumpu/core/instrument.hpp>
#include <securepath/audio/util/audio_data.hpp>

#include <cmath>
#include <algorithm>
#include <cstdio>
#include <filesystem>

using namespace securepath;
using namespace securepath::drum;

static const char* kick_path = TEST_DATA_DIR "/test_kick.wav";

static song make_kick_song(std::uint32_t sample_rate) {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{kick_path});
	s.load_instruments(sample_rate);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);
	sec.tracks()[0].bars()[0].beats.resize(4);
	beat b;
	b.action = beat::hit;
	b.hit_data.volume = volume{false, 1.0f};
	sec.tracks()[0].bars()[0].beats[0] = b;
	return s;
}

TEST_CASE("export writes a wav in the export format", "[export]") {
	export_options opts;
	auto const rate = opts.format.samples_per_second;
	song s = make_kick_song(rate);

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_export.wav").string();
	wav_exporter exporter(file, s, opts);
	while(exporter.process()) {
	}

	audio::audio_data result;
	result.load(file);
	CHECK(result.format() == opts.format);

	auto const bytes = result.data();
	CHECK(std::ranges::any_of(bytes, [](auto v) { return v != 0; }));

	// frame count matches the song duration at the export rate
	auto frames = double(bytes.size()) / 2; // 16-bit mono
	CHECK(std::abs(frames - double(exporter.duration()) * rate) < rate * 0.1);

	std::remove(file.c_str());
}

TEST_CASE("exporting a silent song produces silence, not garbage", "[export]") {
	// pre-fix: peak normalisation divided by zero -> every sample NaN
	export_options opts;
	song s = make_kick_song(opts.format.samples_per_second);
	REQUIRE(!s.track_settings().empty());
	s.track_settings()[0].volume.mute = true;

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_silent.wav").string();
	wav_exporter exporter(file, s, opts);
	while(exporter.process()) {
	}

	audio::audio_data result;
	result.load(file);
	std::remove(file.c_str());

	auto const bytes = result.data();
	REQUIRE(!bytes.empty());
	CHECK(std::ranges::all_of(bytes, [](auto v) { return v == 0; }));
}

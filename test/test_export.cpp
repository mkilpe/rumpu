#include <catch2/catch_all.hpp>

#include "song_fixtures.hpp"

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

TEST_CASE("export writes a wav in the export format", "[export]") {
	export_options opts;
	auto const rate = opts.format.samples_per_second;
	song s = test::make_kick_song(120, rate);

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
	song s = test::make_kick_song(120, opts.format.samples_per_second);
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

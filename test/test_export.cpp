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
#include <optional>

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

TEST_CASE("exporter owns the song snapshot", "[export]") {
	// pre-fix: the mixer kept a pointer to the caller's song, so an exporter
	// outliving the song it was built from read freed memory while mixing
	export_options opts;
	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_owned.wav").string();
	std::optional<wav_exporter> exporter;
	{
		song s = test::make_kick_song(120, opts.format.samples_per_second);
		exporter.emplace(file, std::move(s), opts);
	}
	while(exporter->process()) {
	}

	audio::audio_data result;
	result.load(file);
	std::remove(file.c_str());

	auto const bytes = result.data();
	CHECK(std::ranges::any_of(bytes, [](auto v) { return v != 0; }));
}

TEST_CASE("samples are loaded incrementally with progress", "[export]") {
	export_options opts;
	song s{{}, {4, 4}, {120.0f}};
	auto idx = s.add_instrument(instrument{test::kick_path});
	s.instruments()[idx].add_sample(test::kick_path);
	s.section_order().push_back(s.add_section());

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_loading.wav").string();
	wav_exporter exporter(file, std::move(s), opts);

	CHECK(exporter.is_loading());
	CHECK(exporter.samples_total() == 2);
	CHECK(exporter.samples_loaded() == 0);

	REQUIRE(exporter.process());
	CHECK(exporter.samples_loaded() == 1);
	REQUIRE(exporter.process());
	CHECK(exporter.samples_loaded() == 2);
	CHECK(!exporter.is_loading());

	while(exporter.process()) {
	}
	CHECK(std::filesystem::exists(file));
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

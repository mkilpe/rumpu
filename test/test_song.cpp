#include <catch2/catch_all.hpp>

#include <rumpu/core/song.hpp>
#include <rumpu/core/song_file.hpp>
#include <rumpu/core/serialisation/serialisation.hpp>
#include <securepath/serialisation/util.hpp>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <limits>

using namespace securepath;
using namespace securepath::drum;

TEST_CASE("add_section does not reuse a live section id", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	s.find_section(id2)->set_length(7);

	s.remove_section(id1);
	auto id3 = s.add_section();

	CHECK(id3 != id2);
	REQUIRE(s.find_section(id2) != nullptr);
	CHECK(s.find_section(id2)->length() == 7);
	REQUIRE(s.find_section(id3) != nullptr);
}

TEST_CASE("add_section after removal clones layout from surviving section", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	auto& sec2 = *s.find_section(id2);
	sec2.add_track(0);
	sec2.add_track(1);

	s.remove_section(id1);
	auto id3 = s.add_section();

	REQUIRE(s.find_section(id3) != nullptr);
	CHECK(s.find_section(id3)->tracks().size() == sec2.tracks().size());
	CHECK(s.find_section(id2)->tracks().size() == 2);
}

TEST_CASE("add_section starts from id 1 on an empty song", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	CHECK(id1 == 1);
	s.remove_section(id1);
	CHECK(s.add_section() == 1);
}

TEST_CASE("add_section sizes track_settings to the track count", "[song][track_settings]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_instrument(instrument{});
	s.add_section();
	CHECK(s.track_settings().size() == 2);
}

TEST_CASE("add_track appends a settings entry", "[song][track_settings]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_section();
	REQUIRE(s.track_settings().size() == 1);

	s.add_track(0, "second");
	REQUIRE(s.track_settings().size() == 2);
	for(auto const& [id, sec] : s.sections()) {
		CHECK(sec.tracks().size() == 2);
	}
}

TEST_CASE("remove_instrument erases the matching settings entry", "[song][track_settings]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_instrument(instrument{});
	s.add_section();
	REQUIRE(s.track_settings().size() == 2);
	s.track_settings()[0].volume.value = 0.25f;
	s.track_settings()[1].volume.value = 0.75f;

	s.remove_instrument(0);
	REQUIRE(s.track_settings().size() == 1);
	CHECK(s.track_settings()[0].volume.value == 0.75f);
}

TEST_CASE("sync_track_settings migrates from the first section", "[song][track_settings]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_section();
	auto& trk = s.sections().begin()->second.tracks()[0];
	trk.set_volume({true, 0.5f});
	trk.set_random_seed(1234);

	// simulate a pre-settings file: settings absent after load
	s.track_settings().clear();
	s.sync_track_settings();

	REQUIRE(s.track_settings().size() == 1);
	CHECK(s.track_settings()[0].volume.mute == true);
	CHECK(s.track_settings()[0].volume.value == 0.5f);
	CHECK(s.track_settings()[0].random_seed == 1234);
}

TEST_CASE("track_settings round-trip through song file", "[song][track_settings]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_section();
	REQUIRE(s.track_settings().size() == 1);
	s.track_settings()[0].volume = {true, 0.5f};
	s.track_settings()[0].random_seed = 4242;

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_settings.spd").string();
	save_song_file(file, s);
	song loaded = load_song_file(file);
	std::remove(file.c_str());

	REQUIRE(loaded.track_settings().size() == 1);
	CHECK(loaded.track_settings()[0].volume.mute == true);
	CHECK(loaded.track_settings()[0].volume.value == 0.5f);
	CHECK(loaded.track_settings()[0].random_seed == 4242);
}

static void save_with_version(std::string const& file, int version, song& s) {
	std::ofstream out(file, std::ios_base::binary | std::ios_base::trunc);
	out.write("spd", 3);
	serialisation::asn_der_encoder enc(out);
	serialisation::serialiser ser(enc);
	ser & version & s;
}

TEST_CASE("loader accepts files older than the current version", "[song][serialisation]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	s.add_section();

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_version.spd").string();
	save_with_version(file, 1, s);
	CHECK_NOTHROW(load_song_file(file));
	std::remove(file.c_str());
}

TEST_CASE("loader rejects files newer than the current version", "[song][serialisation]") {
	song s{{}, {4, 4}, {120}};
	s.add_section();

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_version.spd").string();
	save_with_version(file, 3, s);
	CHECK_THROWS_AS(load_song_file(file), std::runtime_error);
	save_with_version(file, 0, s);
	CHECK_THROWS_AS(load_song_file(file), std::runtime_error);
	std::remove(file.c_str());
}

// M4: hostile-but-well-formed files must be rejected at load, not crash at play

static song make_valid_song() {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{});
	auto id = s.add_section();
	s.section_order().push_back(id);
	return s;
}

static void check_load_rejects(song& s) {
	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_validate.spd").string();
	save_song_file(file, s);
	CHECK_THROWS_AS(load_song_file(file), std::runtime_error);
	std::remove(file.c_str());
}

TEST_CASE("validate_song accepts a well-formed song", "[song][validate]") {
	song s = make_valid_song();
	CHECK_NOTHROW(validate_song(s));
}

TEST_CASE("load rejects out-of-range instrument index", "[song][validate]") {
	song s = make_valid_song();
	s.sections().begin()->second.tracks()[0].set_instrument_index(1000);
	check_load_rejects(s);
}

TEST_CASE("load rejects invalid tempo", "[song][validate]") {
	song zero = make_valid_song();
	zero.set_default_tempo({0});
	check_load_rejects(zero);

	song nan = make_valid_song();
	nan.set_default_tempo({std::numeric_limits<float>::quiet_NaN()});
	CHECK_THROWS_AS(validate_song(nan), std::runtime_error);
}

TEST_CASE("load rejects invalid per-bar tempo change", "[song][validate]") {
	song s = make_valid_song();
	s.sections().begin()->second.set_tempo_change(1, tempo{0});
	check_load_rejects(s);
}

TEST_CASE("load rejects invalid time signature", "[song][validate]") {
	song s = make_valid_song();
	s.set_default_time_signature({0, 4});
	check_load_rejects(s);
}

TEST_CASE("load rejects zero-length section", "[song][validate]") {
	song s = make_valid_song();
	s.sections().begin()->second.set_length(0);
	check_load_rejects(s);
}

TEST_CASE("load rejects differing per-section track counts", "[song][validate]") {
	song s = make_valid_song();
	auto id2 = s.add_section();
	s.find_section(id2)->add_track(0);
	check_load_rejects(s);
}

TEST_CASE("load rejects section order with missing ids", "[song][validate]") {
	song s = make_valid_song();
	s.section_order().push_back(99);
	check_load_rejects(s);
}

TEST_CASE("load rejects absurd beat division nesting", "[song][validate]") {
	song s = make_valid_song();
	auto& beats = s.sections().begin()->second.tracks()[0].bars()[0].beats;
	beats.resize(1);
	// 100 nesting levels: enough to exceed the decoder's depth cap, shallow
	// enough that the (recursive) encoder can still write it
	beat* b = &beats[0];
	for(int i = 0; i != 100; ++i) {
		b->division.resize(1);
		b = &b->division[0];
	}
	check_load_rejects(s);
}

TEST_CASE("save to an unwritable path throws and leaves no temp file", "[song][save]") {
	song s = make_valid_song();
	auto file = (std::filesystem::temp_directory_path()
		/ "rumpu_no_such_dir" / "song.spd").string();

	CHECK_THROWS_AS(save_song_file(file, s), std::runtime_error);
	CHECK(!std::filesystem::exists(file + ".tmp"));
}

TEST_CASE("failed save does not touch an existing file", "[song][save]") {
	song s = make_valid_song();
	auto dir = std::filesystem::temp_directory_path() / "rumpu_test_save_dir";
	std::filesystem::create_directory(dir);
	auto file = (dir / "song.spd").string();
	save_song_file(file, s);

	// make the directory unwritable so the temp file cannot be created
	std::filesystem::permissions(dir, std::filesystem::perms::owner_read
		| std::filesystem::perms::owner_exec);
	auto restore = [&] {
		std::filesystem::permissions(dir, std::filesystem::perms::owner_all);
	};

	bool const writable_anyway = [&] {
		// e.g. running as root or on a filesystem without permission support
		std::ofstream probe(file + ".probe");
		bool ok = probe.good();
		probe.close();
		std::error_code ec;
		std::filesystem::remove(file + ".probe", ec);
		return ok;
	}();

	if(!writable_anyway) {
		CHECK_THROWS_AS(save_song_file(file, s), std::runtime_error);
		restore();
		CHECK_NOTHROW(load_song_file(file));
	} else {
		restore();
	}
	std::filesystem::remove_all(dir);
}

TEST_CASE("load accepts moderate beat division nesting", "[song][validate]") {
	song s = make_valid_song();
	auto& beats = s.sections().begin()->second.tracks()[0].bars()[0].beats;
	beats.resize(1);
	beat* b = &beats[0];
	for(int i = 0; i != 4; ++i) {
		b->division.resize(2);
		b = &b->division[0];
	}

	auto file = (std::filesystem::temp_directory_path() / "rumpu_test_nesting.spd").string();
	save_song_file(file, s);
	CHECK_NOTHROW(load_song_file(file));
	std::remove(file.c_str());
}

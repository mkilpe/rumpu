#include <catch2/catch_all.hpp>

#include <rumpu/core/mixer.hpp>
#include <rumpu/core/song.hpp>
#include <rumpu/core/instrument.hpp>

using namespace securepath::drum;

static const char* kick_path = TEST_DATA_DIR "/test_kick.wav";

static void add_kick_pattern(song& s) {
	s.add_instrument(instrument{kick_path});
	s.load_instruments(44100);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);
	beat b;
	b.action = beat::hit;
	b.hit_data.volume = volume{false, 1.0f};
	sec.tracks()[0].bars()[0].beats[0] = b;
}

TEST_CASE("mixer produces no audio for empty song", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	mixer m{s, 44100};
	std::vector<float> buf(4096, 0.0f);
	CHECK(m.process(buf.data(), buf.size()) == 0);
}

TEST_CASE("mixer produces audio for kick song", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	mixer m{s, 44100};
	std::vector<float> buf(4096, 0.0f);
	CHECK(m.process(buf.data(), buf.size()) > 0);
}

TEST_CASE("mixer kick starts at sample 0", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	mixer m{s, 44100};
	std::vector<float> buf(4096, 0.0f);
	m.process(buf.data(), buf.size());
	CHECK(buf[0] != 0.0f);
}

TEST_CASE("mixer play_position advances after processing", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	mixer m{s, 44100};
	std::vector<float> buf(4096, 0.0f);
	m.process(buf.data(), buf.size());
	CHECK(m.play_position() > 0.0f);
}

TEST_CASE("mixer muted track produces no audio", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	s.instruments()[0].set_volume(volume{true, 1.0f});
	mixer m{s, 44100};
	std::vector<float> buf(4096, 0.0f);
	m.process(buf.data(), buf.size());
	for(float v : buf) {
		CHECK(v == 0.0f);
	}
}

TEST_CASE("mixer section-only playback uses given section", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	auto sec_id = s.section_order()[0];
	mixer m{s, sec_id, 44100};
	std::vector<float> buf(4096, 0.0f);
	CHECK(m.process(buf.data(), buf.size()) > 0);
}

TEST_CASE("mixer duration for single section at 120 BPM", "[mixer]") {
	// 4/4 time, 120 BPM, 4 bars = 4 beats/bar * 4 bars / (120 beats/min) * 60 s/min = 8 seconds
	song s{{}, {4, 4}, {120}};
	auto id = s.add_section();
	s.section_order().push_back(id);
	mixer m{s, id, 44100};
	CHECK(m.duration() == Catch::Approx(8.0f).margin(0.01f));
}

TEST_CASE("mixer duration for whole song with multiple sections", "[mixer]") {
	// Two sections of 4 bars each at 120 BPM = 16 seconds
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	s.section_order() = {id1, id2};
	mixer m{s, 44100};
	CHECK(m.duration() == Catch::Approx(16.0f).margin(0.01f));
}

TEST_CASE("mixer duration with tempo change", "[mixer]") {
	// Section 1: 4 bars at 120 BPM = 8s
	// Section 2: 4 bars at 60 BPM = 16s
	// Total = 24s
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	s.find_section(id2)->set_tempo_change(0, tempo{60.0f});
	s.section_order() = {id1, id2};
	mixer m{s, 44100};
	CHECK(m.duration() == Catch::Approx(24.0f).margin(0.01f));
}

TEST_CASE("mixer duration for empty song is zero", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	mixer m{s, 44100};
	CHECK(m.duration() == Catch::Approx(0.0f));
}

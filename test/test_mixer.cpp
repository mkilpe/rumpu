#include <catch2/catch_all.hpp>

#include <rumpu/core/mixer.hpp>
#include <rumpu/core/song.hpp>
#include <rumpu/core/instrument.hpp>

using namespace securepath::drum;

static const char* kick_path = TEST_DATA_DIR "/test_kick.wav";

static void add_kick_pattern(song& s) {
	s.add_instrument(instrument{kick_path});
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

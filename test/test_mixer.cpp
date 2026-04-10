#include <catch2/catch_all.hpp>

#include <rumpu/core/mixer.hpp>
#include <rumpu/core/song.hpp>
#include <rumpu/core/song_file.hpp>
#include <rumpu/core/instrument.hpp>

#include <cstdio>
#include <filesystem>

using namespace securepath::drum;

static const char* kick_path = TEST_DATA_DIR "/test_kick.wav";
static const char* kick2_path = TEST_DATA_DIR "/test_pcm16.wav";

static void add_kick_pattern(song& s) {
	s.add_instrument(instrument{kick_path});
	s.load_instruments(44100);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);
	sec.tracks()[0].bars()[0].beats.resize(4);
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
	auto sec_id = s.section_order()[0];
	s.find_section(sec_id)->tracks()[0].set_volume(volume{true, 1.0f});
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

TEST_CASE("mixer choke in same bar does not kill earlier beat", "[mixer]") {
	// First, find the actual sample length so we can place the choke within it
	instrument inst{kick_path};
	inst.load_samples(44100);
	auto sample_len = inst.sample_to_play().buffer()->size();
	REQUIRE(sample_len > 100);

	// Use high tempo so one beat is shorter than the sample
	// At 960 BPM: samples_per_beat = 44100*60/960 = 2756
	// With 2 divisions: choke at 1378 samples, well within the sample
	float bpm = 44100.0f * 60.0f / (sample_len / 2.0f);
	song s{{}, {4, 4}, {bpm}};
	s.add_instrument(instrument{kick_path});
	s.load_instruments(44100);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);

	auto& beats = sec.tracks()[0].bars()[0].beats;
	beats.resize(4);
	beat hit;
	hit.action = beat::hit;
	hit.hit_data.volume = volume{false, 1.0f};
	beat choke;
	choke.action = beat::stop;
	choke.stop_data.falloff = audio_falloff::create(audio_falloff::immediate);
	beats[0].division = {hit, choke};

	std::uint32_t samples_per_beat = 44100 * 60 / bpm;
	std::uint32_t choke_pos = samples_per_beat / 2;

	mixer m{s, sec_id, 44100};
	std::vector<float> buf(samples_per_beat * 4, 0.0f);
	m.process(buf.data(), buf.size());

	// hit at beat 0 should produce audio
	CHECK(buf[0] != 0.0f);
	// audio should still be playing just before the choke
	CHECK(buf[choke_pos - 1] != 0.0f);
	// after choke, audio should be silenced
	CHECK(buf[choke_pos + 1] == 0.0f);
}

static std::uint32_t add_multi_sample_pattern(song& s) {
	instrument inst{kick_path};
	inst.add_sample(kick2_path);
	s.add_instrument(std::move(inst));
	s.load_instruments(44100);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);
	for(auto& bar : sec.tracks()[0].bars()) {
		for(auto& b : bar.beats) {
			b.action = beat::hit;
			b.hit_data.volume = volume{false, 1.0f};
		}
	}
	return sec_id;
}

static std::vector<float> render_whole_song(song const& s, std::size_t samples) {
	mixer m{s, 44100};
	std::vector<float> buf(samples, 0.0f);
	m.process(buf.data(), buf.size());
	return buf;
}

static std::vector<float> render_section(song const& s, std::uint32_t sec_id, std::size_t samples) {
	mixer m{s, sec_id, 44100};
	std::vector<float> buf(samples, 0.0f);
	m.process(buf.data(), buf.size());
	return buf;
}

TEST_CASE("multi-sample test fixtures have distinct content", "[mixer][random_sample]") {
	// Pre-condition for the random-sample tests: the two fixture WAVs must differ,
	// otherwise we can't tell which sample the mixer picked.
	auto a = load_drum_sample(kick_path);
	auto b = load_drum_sample(kick2_path);
	REQUIRE(a.buffer() != nullptr);
	REQUIRE(b.buffer() != nullptr);
	REQUIRE(*a.buffer() != *b.buffer());
}

TEST_CASE("mixer playback of multi-sample instrument is reproducible across fresh mixers",
		  "[mixer][random_sample]") {
	song s{{}, {4, 4}, {120}};
	add_multi_sample_pattern(s);
	// 4 bars at 120 BPM 4/4 = 8 s = 352800 samples. Render half a second past to be safe.
	std::size_t const n = 44100 * 9;
	auto first = render_whole_song(s, n);
	auto second = render_whole_song(s, n);
	CHECK(first == second);
}

TEST_CASE("mixer single-section playback of multi-sample instrument is reproducible",
		  "[mixer][random_sample]") {
	song s{{}, {4, 4}, {120}};
	auto sec_id = add_multi_sample_pattern(s);
	std::size_t const n = 44100 * 9;
	auto first = render_section(s, sec_id, n);
	auto second = render_section(s, sec_id, n);
	CHECK(first == second);
}

TEST_CASE("mixer produces different audio for repeated section within song_order",
		  "[mixer][random_sample]") {
	// Play the same section twice back-to-back in the song order. The reseed at
	// each section boundary uses sec_order_.size(), so the two visits get
	// different RNG starting states and must produce different audio.
	song s{{}, {4, 4}, {120}};
	auto sec_id = add_multi_sample_pattern(s);
	s.section_order().push_back(sec_id);  // order is now [sec_id, sec_id]

	std::size_t const per_section = 44100 * 8; // 8s per section
	std::size_t const n = per_section * 2;
	auto audio = render_whole_song(s, n);

	std::vector<float> first_visit(audio.begin(), audio.begin() + per_section);
	std::vector<float> second_visit(audio.begin() + per_section, audio.end());
	CHECK(first_visit != second_visit);
}

TEST_CASE("mixer tracks with different seeds pick different sample sequences",
		  "[mixer][random_sample]") {
	// Two independent songs, each with one track, same multi-sample instrument,
	// same hit pattern, but different track seeds. Audio outputs must differ.
	song a{{}, {4, 4}, {120}};
	add_multi_sample_pattern(a);
	a.find_section(a.section_order()[0])->tracks()[0].set_random_seed(1u);

	song b{{}, {4, 4}, {120}};
	add_multi_sample_pattern(b);
	b.find_section(b.section_order()[0])->tracks()[0].set_random_seed(2u);

	std::size_t const n = 44100 * 9;
	auto audio_a = render_whole_song(a, n);
	auto audio_b = render_whole_song(b, n);
	CHECK(audio_a != audio_b);
}

TEST_CASE("mixer single-sample instrument plays identically to before random selection",
		  "[mixer][random_sample]") {
	// Regression: when the instrument has only one sample, the random branch is
	// skipped (samples.size() > 1 is false) and the output must match the old
	// deterministic behaviour. We can't compare against "before the change" here,
	// but we can assert that two fresh mixers produce identical audio (trivially
	// true without RNG) and that the audio is non-silent.
	song s{{}, {4, 4}, {120}};
	add_kick_pattern(s);
	std::size_t const n = 44100 * 9;
	auto first = render_whole_song(s, n);
	auto second = render_whole_song(s, n);
	CHECK(first == second);
	bool has_audio = false;
	for(float v : first) {
		if(v != 0.0f) { has_audio = true; break; }
	}
	CHECK(has_audio);
}

TEST_CASE("track random_seed persists across save/load", "[track][random_sample]") {
	namespace fs = std::filesystem;
	auto tmp_path = fs::path{"rumpu_test_seed_roundtrip.spd"};

	std::uint32_t const pinned_seed = 0xDEADBEEFu;
	{
		song s{{}, {4, 4}, {120}};
		s.add_instrument(instrument{kick_path});
		auto sec_id = s.add_section();
		s.section_order().push_back(sec_id);
		s.find_section(sec_id)->tracks()[0].set_random_seed(pinned_seed);
		save_song_file(tmp_path.string(), s);
	}
	{
		auto loaded = load_song_file(tmp_path.string());
		auto sec_id = loaded.section_order()[0];
		auto const& t = loaded.find_section(sec_id)->tracks()[0];
		CHECK(t.random_seed() == pinned_seed);
	}
	std::error_code ec;
	fs::remove(tmp_path, ec);
}

TEST_CASE("mixer choke stop with falloff silences audio", "[mixer]") {
	song s{{}, {4, 4}, {120}};
	s.add_instrument(instrument{kick_path});
	s.load_instruments(44100);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);

	// bar 0: hit on beat 0
	auto& bar0_beats = sec.tracks()[0].bars()[0].beats;
	bar0_beats.resize(4);
	bar0_beats[0].action = beat::hit;
	bar0_beats[0].hit_data.volume = volume{false, 1.0f};

	// bar 1: choke stop on beat 0 with immediate falloff
	auto& bar1_beats = sec.tracks()[0].bars()[1].beats;
	bar1_beats.resize(4);
	bar1_beats[0].action = beat::stop;
	bar1_beats[0].stop_data.falloff = audio_falloff::create(audio_falloff::immediate);

	// 4/4 at 120bpm, 44100hz: samples_per_bar = 88200
	mixer m{s, sec_id, 44100};
	std::vector<float> buf(176400, 0.0f);
	m.process(buf.data(), buf.size());

	// bar 0 should produce audio (hit at sample 0)
	CHECK(buf[0] != 0.0f);
	// bar 1 choke fires at start of bar 1 (sample 88200), audio should be silenced after
	CHECK(buf[88201] == 0.0f);
}

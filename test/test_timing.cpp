#include <catch2/catch_all.hpp>

#include <rumpu/core/audio_falloff.hpp>
#include <rumpu/core/bar_timing.hpp>
#include <rumpu/core/section.hpp>
#include <rumpu/core/tempo.hpp>
#include <rumpu/core/time_signature.hpp>
#include <rumpu/core/volume.hpp>

using namespace securepath::drum;
using Catch::Approx;

static std::uint32_t samples_per_bar(time_signature timing, time_signature default_timing, tempo t, std::uint32_t sample_rate) {
	bar_timing bt{default_timing, t, std::nullopt};
	bt.current_timing = timing;
	return bt.bar_samples(sample_rate);
}

TEST_CASE("samples_per_bar 4/4 120bpm 44100hz", "[timing]") {
	time_signature ts{4, 4};
	CHECK(samples_per_bar(ts, ts, tempo{120}, 44100) == 88200);
}

TEST_CASE("samples_per_bar 3/4 120bpm 44100hz", "[timing]") {
	time_signature def{4, 4};
	CHECK(samples_per_bar({3, 4}, def, tempo{120}, 44100) == 49612);
}

TEST_CASE("samples_per_bar scales with tempo", "[timing]") {
	time_signature ts{4, 4};
	CHECK(samples_per_bar(ts, ts, tempo{240}, 44100) < samples_per_bar(ts, ts, tempo{120}, 44100));
}

TEST_CASE("samples_per_bar scales with sample rate", "[timing]") {
	time_signature ts{4, 4};
	CHECK(samples_per_bar(ts, ts, tempo{120}, 88200) > samples_per_bar(ts, ts, tempo{120}, 44100));
}

TEST_CASE("tempo_slide bar_delta", "[tempo_slide]") {
	tempo_slide slide{0, 4, 10.0f};
	CHECK(slide.bar_delta() == 2.5f);
}

TEST_CASE("tempo_slide is_active", "[tempo_slide]") {
	tempo_slide slide{2, 6, 10.0f};
	CHECK(!slide.is_active(1));
	CHECK(slide.is_active(2));
	CHECK(slide.is_active(5));
	CHECK(!slide.is_active(6));
}

TEST_CASE("time_signature validity", "[time_signature]") {
	CHECK(!time_signature{}.is_valid());
	CHECK(time_signature{4, 4}.is_valid());
	CHECK(time_signature{3, 4}.is_valid());
	CHECK(!time_signature{0, 4}.is_valid());
	CHECK(!time_signature{4, 0}.is_valid());
}

TEST_CASE("audio_falloff immediate player returns 0", "[audio_falloff]") {
	auto def = audio_falloff::create(audio_falloff::immediate);
	auto p = def->create_player(0.5f);
	CHECK(p->factor(0.5f) == 0.0f);
	CHECK(p->is_done());
}

TEST_CASE("audio_falloff linear player decreases from 1 to 0", "[audio_falloff]") {
	// 1 beat at 0.5s per beat = 0.5s duration
	auto def = audio_falloff::create(audio_falloff::linear, 1.0f);
	auto p = def->create_player(0.5f);
	CHECK(p->factor(0.0f) == Approx(1.0f));
	CHECK(p->factor(0.5f) == Approx(0.0f));
}

TEST_CASE("audio_falloff player is_done after full duration", "[audio_falloff]") {
	auto def = audio_falloff::create(audio_falloff::linear, 1.0f);
	auto p = def->create_player(1.0f);
	CHECK(!p->is_done());
	p->factor(1.0f);
	CHECK(p->is_done());
}

TEST_CASE("audio_falloff exponential player factor", "[audio_falloff]") {
	auto def = audio_falloff::create(audio_falloff::exponential, 1.0f);
	auto p = def->create_player(1.0f);
	CHECK(!p->is_done());
	float mid = p->factor(0.5f);
	CHECK(mid > 0.0f);
	CHECK(mid < 1.0f);
	p->factor(0.5f);
	CHECK(p->is_done());
}

TEST_CASE("audio_falloff clone preserves definition", "[audio_falloff]") {
	auto def = audio_falloff::create(audio_falloff::linear, 2.0f);
	auto c = def->clone();
	CHECK(c->type() == audio_falloff::linear);
	CHECK(c->duration_beats() == 2.0f);
}

TEST_CASE("audio_falloff players are independent", "[audio_falloff]") {
	auto def = audio_falloff::create(audio_falloff::linear, 1.0f);
	auto p1 = def->create_player(1.0f);
	auto p2 = def->create_player(1.0f);
	p1->factor(1.0f);
	CHECK(p1->is_done());
	CHECK(!p2->is_done());
}

TEST_CASE("volume_slide bar_delta", "[volume_slide]") {
	volume_slide vs{0, 4, 1.0f};
	CHECK(vs.bar_delta() == 0.25f);
}

TEST_CASE("volume_slide is_active", "[volume_slide]") {
	volume_slide vs{2, 6, 1.0f};
	CHECK(!vs.is_active(1));
	CHECK(vs.is_active(2));
	CHECK(vs.is_active(5));
	CHECK(!vs.is_active(6));
}

// slides must accumulate on every bar of [begin, end), including bars
// without an explicit change record.

static std::vector<float> walk_tempi(bar_timing t, section const& sec) {
	std::vector<float> tempi;
	t.new_section();
	for(std::uint32_t bar = 0; bar != sec.length(); ++bar) {
		t.begin_bar(sec, bar);
		tempi.push_back(t.current_tempo.value);
	}
	return tempi;
}

TEST_CASE("bar_samples clamps a bar longer than the sample counter", "[timing]") {
	// each field is within the limits the loader accepts, but combined they
	// give ~5.5e15 samples: the cast to uint32 must clamp, not overflow
	section sec{1};
	sec.changes()[0].timing_change = time_signature{128, 1};

	bar_timing t{{1, 128}, tempo{1.0f}, std::nullopt};
	t.new_section();
	t.begin_bar(sec, 0);
	CHECK(t.bar_samples(44100) == std::numeric_limits<std::uint32_t>::max());
}

TEST_CASE("bar_samples never returns a zero-length bar", "[timing]") {
	// the mirror extreme rounds to zero samples, which would stall the mixer's
	// render loop
	section sec{1};
	sec.changes()[0].timing_change = time_signature{1, 128};

	bar_timing t{{128, 1}, tempo{9999.0f}, std::nullopt};
	t.new_section();
	t.begin_bar(sec, 0);
	CHECK(t.bar_samples(44100) >= 1);
}

TEST_CASE("tempo slide accumulates on bars without change records", "[timing][slide]") {
	section sec{5};
	sec.changes()[1].tempo_slide_change = tempo_slide{1, 4, 30.0f};

	bar_timing t{{4, 4}, tempo{120}, std::nullopt};
	auto tempi = walk_tempi(t, sec);

	REQUIRE(tempi.size() == 5);
	CHECK(tempi[0] == Approx(120.0f));
	CHECK(tempi[1] == Approx(130.0f));
	CHECK(tempi[2] == Approx(140.0f)); // no record on this bar: must still advance
	CHECK(tempi[3] == Approx(150.0f));
	CHECK(tempi[4] == Approx(150.0f)); // slide ended
}

TEST_CASE("global tempo slide applies on every bar", "[timing][slide]") {
	section sec{4};
	bar_timing t{{4, 4}, tempo{120}, delta_tempo{5.0f}};
	auto tempi = walk_tempi(t, sec);

	CHECK(tempi[0] == Approx(125.0f));
	CHECK(tempi[3] == Approx(140.0f));
}

TEST_CASE("explicit tempo change overrides the slide for that bar", "[timing][slide]") {
	section sec{4};
	sec.changes()[0].tempo_slide_change = tempo_slide{0, 4, 40.0f};
	sec.changes()[2].tempo_change = tempo{100};

	bar_timing t{{4, 4}, tempo{120}, std::nullopt};
	auto tempi = walk_tempi(t, sec);

	CHECK(tempi[0] == Approx(130.0f));
	CHECK(tempi[1] == Approx(140.0f));
	CHECK(tempi[2] == Approx(100.0f)); // explicit set, no slide delta
	CHECK(tempi[3] == Approx(110.0f)); // slide resumes
}

TEST_CASE("slide-driven tempo is clamped", "[timing][slide]") {
	section sec{3};
	sec.changes()[0].tempo_slide_change = tempo_slide{0, 3, -600.0f};

	bar_timing t{{4, 4}, tempo{120}, std::nullopt};
	auto tempi = walk_tempi(t, sec);

	CHECK(tempi[2] == Approx(1.0f));
}

TEST_CASE("new_section clears an active slide but keeps the tempo", "[timing][slide]") {
	section sec{2};
	sec.changes()[0].tempo_slide_change = tempo_slide{0, 8, 80.0f};

	bar_timing t{{4, 4}, tempo{120}, std::nullopt};
	t.new_section();
	t.begin_bar(sec, 0);
	t.begin_bar(sec, 1);
	CHECK(t.current_tempo.value == Approx(140.0f));

	t.new_section();
	section plain{2};
	t.begin_bar(plain, 0);
	CHECK(t.current_tempo.value == Approx(140.0f)); // slide gone, tempo carried
}

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

#include <catch2/catch_all.hpp>

#include <rumpu/core/tempo.hpp>
#include <rumpu/core/time_signature.hpp>
#include <rumpu/core/volume.hpp>

using namespace securepath::drum;

static std::uint32_t samples_per_bar(time_signature timing, time_signature default_timing, tempo t, std::uint32_t sample_rate) {
	double timing_multiplier = default_timing.beat_type() * timing.beats_in_bar()
		/ double(default_timing.beats_in_bar() * timing.beat_type());
	return 60 * timing_multiplier * sample_rate * timing.beats_in_bar() / t.value;
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

TEST_CASE("audio_falloff immediate always returns 0", "[audio_falloff]") {
	audio_falloff f{audio_falloff::immediate, 0.0f, 1.0f};
	CHECK(f.factor(0.5f) == 0.0f);
	CHECK(f.factor(0.5f) == 0.0f);
}

TEST_CASE("audio_falloff linear factor decreases from 1 to 0", "[audio_falloff]") {
	audio_falloff f{audio_falloff::linear, 0.0f, 1.0f};
	CHECK(f.factor(0.0f) == Catch::Approx(1.0f));
	CHECK(f.factor(1.0f) == Catch::Approx(0.0f));
}

TEST_CASE("audio_falloff is_done after full duration", "[audio_falloff]") {
	audio_falloff f{audio_falloff::linear, 0.0f, 1.0f};
	CHECK(!f.is_done());
	f.factor(1.0f);
	CHECK(f.is_done());
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

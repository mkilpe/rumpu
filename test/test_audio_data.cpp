#include <catch2/catch_all.hpp>

#include <securepath/audio/util/audio_data.hpp>

#include <cstring>
#include <vector>

using namespace securepath;
using namespace securepath::audio;
using Catch::Approx;

static octet_vector to_bytes(std::vector<float> const& v) {
	octet_vector out(v.size() * sizeof(float));
	std::memcpy(out.data(), v.data(), out.size());
	return out;
}

static std::vector<float> to_floats(octet_vector const& v) {
	std::vector<float> out(v.size() / sizeof(float));
	std::memcpy(out.data(), v.data(), v.size());
	return out;
}

static audio_format float_mono(std::uint32_t rate) {
	return {audio::float_t, 1, 32, rate};
}

// values stay within [-1, 1]: write_sample clamps on encode
static std::vector<float> ramp(std::size_t n) {
	std::vector<float> v(n);
	for(std::size_t i = 0; i != n; ++i) {
		v[i] = i / 256.0f;
	}
	return v;
}

TEST_CASE("resample upsamples with linear interpolation", "[audio_data][resample]") {
	audio_data d{float_mono(22050), to_bytes(ramp(100))};
	d.resample(float_mono(44100));

	CHECK(d.format().samples_per_second == 44100);
	auto out = to_floats(d.data());
	REQUIRE(out.size() == 200);
	CHECK(out[0] == Approx(0.0f));
	CHECK(out[1] == Approx(0.5f / 256));
	CHECK(out[2] == Approx(1.0f / 256));
	CHECK(out[199] == Approx(99.0f / 256));
}

TEST_CASE("resample downsamples to matching duration", "[audio_data][resample]") {
	audio_data d{float_mono(44100), to_bytes(ramp(200))};
	d.resample(float_mono(22050));

	auto out = to_floats(d.data());
	REQUIRE(out.size() == 100);
	CHECK(out[1] == Approx(2.0f / 256));
	CHECK(out[99] == Approx(198.0f / 256));
}

TEST_CASE("resample preserves duration for non-integer ratios", "[audio_data][resample]") {
	std::size_t const src_frames = 1000;
	audio_data d{float_mono(22050), to_bytes(std::vector<float>(src_frames, 0.25f))};
	d.resample(float_mono(48000));

	auto out = to_floats(d.data());
	double expected = double(src_frames) * 48000 / 22050;
	CHECK(std::abs(double(out.size()) - expected) <= 1.0);
	for(auto v : out) {
		REQUIRE(v == Approx(0.25f));
	}
}

TEST_CASE("resample without rate change keeps frame count", "[audio_data][resample]") {
	// stereo 16-bit, frames (L,R): (0,16384), (-16384,0)
	std::vector<std::int16_t> src{0, 16384, -16384, 0};
	octet_vector bytes(src.size() * sizeof(std::int16_t));
	std::memcpy(bytes.data(), src.data(), bytes.size());

	audio_data d{{short_t, 2, 16, 44100}, std::move(bytes)};
	d.resample(float_mono(44100));

	auto out = to_floats(d.data());
	REQUIRE(out.size() == 2);
	CHECK(out[0] == Approx(0.25f));  // (0 + 0.5) / 2
	CHECK(out[1] == Approx(-0.25f)); // (-0.5 + 0) / 2
}

TEST_CASE("resample rejects zero sample rates", "[audio_data][resample]") {
	audio_data zero_src{float_mono(0), to_bytes(ramp(10))};
	CHECK_THROWS_AS(zero_src.resample(float_mono(44100)), invalid_format);

	audio_data zero_dst{float_mono(44100), to_bytes(ramp(10))};
	CHECK_THROWS_AS(zero_dst.resample(float_mono(0)), invalid_format);
}

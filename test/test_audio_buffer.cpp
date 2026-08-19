#include <catch2/catch_all.hpp>

#include <securepath/audio/audio_lib/audio_buffer.hpp>

#include <cstring>
#include <stdexcept>

using namespace securepath;
using namespace securepath::audio;
using Catch::Approx;

static audio_format float_fmt() { return {audio::float_t, 1, 32, 44100}; }
static audio_format short_fmt() { return {audio::short_t, 1, 16, 44100}; }

static audio_buffer make_float_buffer(std::vector<float> const& v) {
	octet_vector data(v.size() * sizeof(float));
	std::memcpy(data.data(), v.data(), data.size());
	return audio_buffer{float_fmt(), data};
}

static audio_buffer make_short_buffer(std::vector<std::int16_t> const& v) {
	octet_vector data(v.size() * sizeof(std::int16_t));
	std::memcpy(data.data(), v.data(), data.size());
	return audio_buffer{short_fmt(), data};
}

TEST_CASE("audio_buffer float mix adds samples", "[audio_buffer]") {
	// the old integer-based mix truncated float samples to whole numbers and
	// clamped negatives away (lower bound was the smallest positive float)
	auto a = make_float_buffer({0.5f, -0.5f, 0.25f});
	auto b = make_float_buffer({0.25f, -0.25f, -0.75f});

	a.mix(0, b, 0, 3, 1.0);
	auto const* p = a.begin<float>();
	CHECK(p[0] == Approx(0.75f));
	CHECK(p[1] == Approx(-0.75f));
	CHECK(p[2] == Approx(-0.5f));
}

TEST_CASE("audio_buffer float scale keeps negative values", "[audio_buffer]") {
	auto a = make_float_buffer({0.5f, -0.5f});
	a.scale(0, 2, 0.5);
	auto const* p = a.begin<float>();
	CHECK(p[0] == Approx(0.25f));
	CHECK(p[1] == Approx(-0.25f));
}

TEST_CASE("audio_buffer int16 mix and scale clamp instead of overflowing", "[audio_buffer]") {
	auto a = make_short_buffer({30000, -30000});
	auto b = make_short_buffer({30000, -30000});

	a.mix(0, b, 0, 2, 1.0);
	auto const* p = a.begin<std::int16_t>();
	CHECK(p[0] == 32767);
	CHECK(p[1] == -32768);

	auto c = make_short_buffer({30000});
	c.scale(0, 1, 2.0);
	CHECK(c.begin<std::int16_t>()[0] == 32767);
}

TEST_CASE("audio_buffer mix rejects mismatched sample types", "[audio_buffer]") {
	auto a = make_float_buffer({0.5f});
	auto b = make_short_buffer({100});
	CHECK_THROWS_AS(a.mix(0, b, 0, 1, 1.0), std::invalid_argument);
}

TEST_CASE("audio_buffer rejects inconsistent construction", "[audio_buffer]") {
	// data size not a multiple of the sample size: the old code rounded the
	// sample count down and then copied the full data over the end
	octet_vector five_bytes(5);
	CHECK_THROWS_AS((audio_buffer{float_fmt(), five_bytes}), std::invalid_argument);

	// bits_per_sample disagreeing with the storage type desyncs size math
	CHECK_THROWS_AS((audio_buffer{{audio::char_t, 1, 32, 44100}, std::size_t(16)}), std::invalid_argument);
	CHECK_THROWS_AS((audio_buffer{{audio::float_t, 1, 16, 44100}, std::size_t(16)}), std::invalid_argument);
}

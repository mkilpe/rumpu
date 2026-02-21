#include <catch2/catch_all.hpp>

#include <rumpu/core/drum_sample.hpp>

using namespace securepath::drum;

static const char* kick_path = TEST_DATA_DIR "/test_kick.wav";

TEST_CASE("drum_sample loads from file", "[drum_sample]") {
	drum_sample s{kick_path};
	CHECK(s.source_file() == kick_path);
	REQUIRE_NOTHROW(s.load_sample(44100));
	CHECK(s.buffer() != nullptr);
}

TEST_CASE("drum_sample buffer is non-silent", "[drum_sample]") {
	auto s = load_drum_sample(kick_path);
	bool has_nonzero = false;
	for(float v : *s.buffer()) {
		if(v != 0.0f) { has_nonzero = true; break; }
	}
	CHECK(has_nonzero);
}

TEST_CASE("drum_sample peak amplitude is in valid range", "[drum_sample]") {
	auto s = load_drum_sample(kick_path);
	for(float v : *s.buffer()) {
		CHECK(v >= -1.0f);
		CHECK(v <= 1.0f);
	}
}

TEST_CASE("drum_sample reloads at different sample rate", "[drum_sample]") {
	drum_sample s{kick_path};
	s.load_sample(44100);
	s.load_sample(22050);
	CHECK(s.sample_rate() == 22050);
}

TEST_CASE("drum_sample throws on empty path", "[drum_sample]") {
	drum_sample s;
	CHECK_THROWS_AS(s.load_sample(44100), std::runtime_error);
}

TEST_CASE("load_drum_sample helper loads and returns buffer", "[drum_sample]") {
	auto s = load_drum_sample(kick_path);
	CHECK(s.buffer() != nullptr);
}

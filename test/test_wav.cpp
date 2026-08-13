#include <catch2/catch_all.hpp>

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/audio/util/detail/wav.hpp>

#include <cstring>
#include <filesystem>

namespace audio = securepath::audio;

static std::string test_file(std::string const& name) {
	return (std::filesystem::path(TEST_DATA_DIR) / name).string();
}

static audio::audio_data make_test_data(audio::audio_format fmt, std::vector<float> const& samples) {
	std::size_t stride = fmt.bits_per_sample / 8;
	securepath::octet_vector data(samples.size() * stride);
	auto* dst = data.data();
	for (float v : samples) {
		v = std::clamp(v, -1.0f, 1.0f);
		switch (fmt.type) {
		case audio::short_t: {
			auto s = static_cast<std::int16_t>(v * 32767);
			std::memcpy(dst, &s, sizeof(s));
			break;
		}
		case audio::float_t: {
			std::memcpy(dst, &v, sizeof(v));
			break;
		}
		default:
			break;
		}
		dst += stride;
	}
	return audio::audio_data{fmt, std::move(data)};
}

TEST_CASE("wav round-trip 16-bit PCM", "[wav]") {
	audio::audio_format fmt{audio::short_t, 1, 16, 44100, std::endian::little};
	std::vector<float> samples = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f};
	auto ad = make_test_data(fmt, samples);

	auto path = test_file("test_pcm16.wav");
	ad.save(path);

	audio::audio_data loaded;
	loaded.load(path);

	CHECK(loaded.format().type == audio::short_t);
	CHECK(loaded.format().bits_per_sample == 16);
	CHECK(loaded.format().channels == 1);
	CHECK(loaded.format().samples_per_second == 44100);
	CHECK(loaded.data().size() == samples.size() * 2);
}

TEST_CASE("wav round-trip 32-bit float", "[wav]") {
	audio::audio_format fmt{audio::float_t, 1, 32, 44100, std::endian::little};
	std::vector<float> samples = {0.0f, 0.5f, -0.5f, 1.0f, -1.0f, 0.25f};
	auto ad = make_test_data(fmt, samples);

	auto path = test_file("test_float32.wav");
	ad.save(path);

	audio::audio_data loaded;
	loaded.load(path);

	CHECK(loaded.format().type == audio::float_t);
	CHECK(loaded.format().bits_per_sample == 32);
	CHECK(loaded.format().channels == 1);
	CHECK(loaded.format().samples_per_second == 44100);

	// float round-trip should be exact
	auto const& data = loaded.data();
	for (std::size_t i = 0; i < samples.size(); ++i) {
		float v;
		std::memcpy(&v, data.data() + i * sizeof(float), sizeof(float));
		CHECK(v == samples[i]);
	}
}

TEST_CASE("wav convert 16-bit to 32-bit float", "[wav]") {
	audio::audio_format src_fmt{audio::short_t, 1, 16, 44100, std::endian::little};
	std::vector<float> samples = {0.0f, 0.5f, -0.5f};
	auto ad = make_test_data(src_fmt, samples);

	audio::audio_format dst_fmt{audio::float_t, 1, 32, 44100, std::endian::little};
	auto path = test_file("test_16to32.wav");
	ad.save(path, dst_fmt);

	audio::audio_data loaded;
	loaded.load(path);

	CHECK(loaded.format().type == audio::float_t);
	CHECK(loaded.format().bits_per_sample == 32);

	auto const& data = loaded.data();
	for (std::size_t i = 0; i < samples.size(); ++i) {
		float v;
		std::memcpy(&v, data.data() + i * sizeof(float), sizeof(float));
		CHECK(v == Catch::Approx(samples[i]).margin(0.001f));
	}
}

TEST_CASE("wav convert 32-bit float to 16-bit", "[wav]") {
	audio::audio_format src_fmt{audio::float_t, 1, 32, 44100, std::endian::little};
	std::vector<float> samples = {0.0f, 0.5f, -0.5f};
	auto ad = make_test_data(src_fmt, samples);

	audio::audio_format dst_fmt{audio::short_t, 1, 16, 44100, std::endian::little};
	auto path = test_file("test_32to16.wav");
	ad.save(path, dst_fmt);

	audio::audio_data loaded;
	loaded.load(path);

	CHECK(loaded.format().type == audio::short_t);
	CHECK(loaded.format().bits_per_sample == 16);

	// 16-bit has limited precision
	auto const& data = loaded.data();
	for (std::size_t i = 0; i < samples.size(); ++i) {
		std::int16_t s;
		std::memcpy(&s, data.data() + i * sizeof(std::int16_t), sizeof(s));
		float v = s / 32768.0f;
		CHECK(v == Catch::Approx(samples[i]).margin(0.001f));
	}
}

static audio::riff::riff_fmt_data make_fmt(std::uint16_t audio_format, std::uint16_t channels,
	std::uint32_t sample_rate, std::uint16_t bits) {
	audio::riff::riff_fmt_data fmt;
	fmt.audio_format = audio_format;
	fmt.channels = channels;
	fmt.sample_rate = sample_rate;
	fmt.bits_per_sample = bits;
	fmt.byte_rate = sample_rate * channels * bits / 8;
	fmt.block_align = static_cast<std::uint16_t>(channels * bits / 8);
	return fmt;
}

TEST_CASE("wav format validation accepts supported formats", "[wav][validate]") {
	CHECK_NOTHROW(audio::validate_wav_format(make_fmt(1, 1, 44100, 8), 16));
	CHECK_NOTHROW(audio::validate_wav_format(make_fmt(1, 2, 44100, 16), 16));
	CHECK_NOTHROW(audio::validate_wav_format(make_fmt(1, 1, 48000, 24), 15));
	CHECK_NOTHROW(audio::validate_wav_format(make_fmt(3, 2, 44100, 32), 16));
}

TEST_CASE("wav format validation rejects malformed formats", "[wav][validate]") {
	// zero channels: would divide by zero in resample (C4)
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 0, 44100, 16), 0), audio::invalid_format);
	// zero sample rate: would divide by zero in timing conversions (C4)
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 1, 0, 16), 16), audio::invalid_format);
	// float below 32 bits: float decode always reads 4 bytes -> heap over-read (C5)
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(3, 1, 44100, 8), 8), audio::invalid_format);
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(3, 1, 44100, 16), 16), audio::invalid_format);
	// unsupported format tag e.g. WAVE_FORMAT_EXTENSIBLE / compressed (M6)
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(0xFFFE, 1, 44100, 16), 16), audio::invalid_format);
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(2, 1, 44100, 16), 16), audio::invalid_format);
	// 32-bit integer PCM has no decoder and would be read as noise (M6)
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 1, 44100, 32), 16), audio::invalid_format);
	// data size not a multiple of the frame size
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 2, 44100, 16), 5), audio::invalid_format);
}

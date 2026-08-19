#include <catch2/catch_all.hpp>

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/audio/util/detail/wav.hpp>

#include <cstring>
#include <filesystem>
#include <optional>
#include <sstream>

namespace audio = securepath::audio;

// output files: written into the build tree, never the source tree
static std::string test_file(std::string const& name) {
	return (std::filesystem::path(TEST_OUT_DIR) / name).string();
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
	// zero channels: would divide by zero in resample
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 0, 44100, 16), 0), audio::invalid_format);
	// zero sample rate: would divide by zero in timing conversions
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 1, 0, 16), 16), audio::invalid_format);
	// float below 32 bits: float decode always reads 4 bytes -> heap over-read
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(3, 1, 44100, 8), 8), audio::invalid_format);
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(3, 1, 44100, 16), 16), audio::invalid_format);
	// unsupported format tag e.g. WAVE_FORMAT_EXTENSIBLE / compressed
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(0xFFFE, 1, 44100, 16), 16), audio::invalid_format);
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(2, 1, 44100, 16), 16), audio::invalid_format);
	// 32-bit integer PCM has no decoder and would be read as noise
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 1, 44100, 32), 16), audio::invalid_format);
	// data size not a multiple of the frame size
	CHECK_THROWS_AS(audio::validate_wav_format(make_fmt(1, 2, 44100, 16), 5), audio::invalid_format);
}

// -- chunk-level parser robustness: craft raw byte streams --

static void put_bytes(securepath::octet_vector& v, void const* p, std::size_t n) {
	auto const* b = static_cast<std::uint8_t const*>(p);
	v.insert(v.end(), b, b + n);
}

static void put_u32(securepath::octet_vector& v, std::uint32_t x) {
	put_bytes(v, &x, 4); // little-endian hosts only, as the rest of the tests
}

static void put_chunk(securepath::octet_vector& v, char const (&id)[5],
                      securepath::octet_vector const& payload,
                      std::optional<std::uint32_t> declared_size = std::nullopt) {
	put_bytes(v, id, 4);
	put_u32(v, declared_size.value_or(static_cast<std::uint32_t>(payload.size())));
	put_bytes(v, payload.data(), payload.size());
}

static securepath::octet_vector fmt_payload_pcm8_mono() {
	securepath::octet_vector p;
	std::uint16_t const audio_format = 1, channels = 1, block_align = 1, bits = 8;
	std::uint32_t const rate = 44100, byte_rate = 44100;
	put_bytes(p, &audio_format, 2);
	put_bytes(p, &channels, 2);
	put_u32(p, rate);
	put_u32(p, byte_rate);
	put_bytes(p, &block_align, 2);
	put_bytes(p, &bits, 2);
	return p;
}

// chunks = concatenated chunk bytes following the WAVE tag
static securepath::octet_vector raw_wav(securepath::octet_vector const& chunks) {
	securepath::octet_vector v;
	put_bytes(v, "RIFF", 4);
	put_u32(v, static_cast<std::uint32_t>(4 + chunks.size()));
	put_bytes(v, "WAVE", 4);
	put_bytes(v, chunks.data(), chunks.size());
	return v;
}

static void load_raw(securepath::octet_vector const& bytes, securepath::octet_vector& out) {
	std::istringstream in(std::string(bytes.begin(), bytes.end()), std::ios_base::binary);
	audio::wav w(out);
	w.load(in);
}

TEST_CASE("wav rejects huge declared data chunk without allocating it", "[wav][chunks]") {
	// a 4 GB declared size in a tiny file must throw, not allocate
	securepath::octet_vector chunks;
	put_chunk(chunks, "fmt ", fmt_payload_pcm8_mono());
	put_chunk(chunks, "data", {1, 2, 3, 4}, 0xF0000000u);

	securepath::octet_vector out;
	CHECK_THROWS_AS(load_raw(raw_wav(chunks), out), audio::invalid_format);
}

TEST_CASE("wav skips odd-sized chunks with their pad byte", "[wav][chunks]") {
	// an odd LIST chunk is padded to word alignment; the parser must skip
	// the pad or every following chunk header is misread
	securepath::octet_vector chunks;
	put_chunk(chunks, "LIST", {'I', 'N', 'F'});
	chunks.push_back(0); // pad byte
	put_chunk(chunks, "fmt ", fmt_payload_pcm8_mono());
	put_chunk(chunks, "data", {10, 20, 30, 40});

	securepath::octet_vector out;
	CHECK_NOTHROW(load_raw(raw_wav(chunks), out));
	CHECK(out == securepath::octet_vector{10, 20, 30, 40});
}

TEST_CASE("wav truncated inputs throw invalid_format", "[wav][chunks]") {
	securepath::octet_vector out;

	// cut off inside the RIFF header
	securepath::octet_vector header_only{'R', 'I', 'F', 'F', 0, 0};
	CHECK_THROWS_AS(load_raw(header_only, out), audio::invalid_format);

	// fmt chunk declaring fewer bytes than the format struct needs
	{
		securepath::octet_vector chunks;
		put_chunk(chunks, "fmt ", {1, 0, 1, 0, 0, 0}, std::nullopt);
		put_chunk(chunks, "data", {1, 2});
		CHECK_THROWS_AS(load_raw(raw_wav(chunks), out), audio::invalid_format);
	}

	// file ends in the middle of a chunk header
	{
		securepath::octet_vector chunks;
		put_chunk(chunks, "fmt ", fmt_payload_pcm8_mono());
		put_chunk(chunks, "data", {1, 2, 3, 4});
		put_bytes(chunks, "LI", 2); // partial next header
		CHECK_THROWS_AS(load_raw(raw_wav(chunks), out), audio::invalid_format);
	}

	// data chunk declaring more bytes than the file has
	{
		securepath::octet_vector chunks;
		put_chunk(chunks, "fmt ", fmt_payload_pcm8_mono());
		put_chunk(chunks, "data", {1, 2, 3, 4}, 400);
		CHECK_THROWS_AS(load_raw(raw_wav(chunks), out), audio::invalid_format);
	}
}

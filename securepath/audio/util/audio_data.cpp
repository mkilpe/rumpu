#include "audio_data.hpp"
#include "detail/wav.hpp"

#include <bit>
#include <cstring>
#include <fstream>
#include <vector>

namespace securepath::audio {

static std::string file_extension(std::string const& filename) {
	auto p = filename.find_last_of('.');
	return p != std::string::npos ? filename.substr(p+1) : std::string();
}

audio_data::audio_data(audio::audio_format format, octet_vector data)
: format_(format)
, data_(std::move(data))
{
}

audio_data::audio_data(audio::audio_buffer const& buffer)
: format_(buffer.format())
, data_(buffer.begin<std::uint8_t>(), buffer.begin<std::uint8_t>() + buffer.size())
{
}

static_assert(sizeof(float) == 4);

template<typename T>
static T read_value(std::uint8_t const* src, std::endian endian) {
	T v;
	std::memcpy(&v, src, sizeof(T));
	if (endian != std::endian::native) {
		v = std::byteswap(v);
	}
	return v;
}

template<typename T>
static void write_value(std::uint8_t* dst, T v, std::endian endian) {
	if (endian != std::endian::native) {
		v = std::byteswap(v);
	}
	std::memcpy(dst, &v, sizeof(T));
}

static float read_sample(std::uint8_t const* src, audio::audio_format const& fmt) {
	switch(fmt.type) {
	case audio::char_t:  return static_cast<std::int8_t>(*src) / 128.0f;
	case audio::uchar_t: return (*src - 128) / 128.0f;
	case audio::short_t: return read_value<std::int16_t>(src, fmt.endian) / 32768.0f;
	case audio::float_t: return std::bit_cast<float>(read_value<std::uint32_t>(src, fmt.endian));
	}
	return 0.f;
}

static void write_sample(std::uint8_t* dst, float v, audio::audio_format const& fmt) {
	v = std::clamp(v, -1.0f, 1.0f);
	switch(fmt.type) {
	case audio::char_t:  *reinterpret_cast<std::int8_t*>(dst) = std::int8_t(v * 127); break;
	case audio::uchar_t: *dst = std::uint8_t((v + 1.0f) * 127.5f); break;
	case audio::short_t: write_value<std::int16_t>(dst, std::int16_t(v * 32767), fmt.endian); break;
	case audio::float_t: write_value<std::uint32_t>(dst, std::bit_cast<std::uint32_t>(v), fmt.endian); break;
	}
}

static float mix_channels(float const* channels, std::uint32_t count) {
	float v = 0.f;
	for(std::uint32_t i = 0; i != count; ++i) {
		v += channels[i];
	}
	return v / float(count);
}

static float remap_channel(float const* src_channels, std::uint32_t src_ch, std::uint32_t dst_ch, std::uint32_t ch) {
	if(dst_ch < src_ch) {
		return mix_channels(src_channels, src_ch);
	}
	if(ch < src_ch) {
		return src_channels[ch];
	}
	return src_channels[0];
}

void audio_data::resample(audio::audio_format const& target) {
	constexpr size_t max_channels = 16;
	if(format_.channels >= max_channels) {
		throw std::runtime_error("Too many channels, only 16 supported");
	}

	std::uint32_t src_stride = format_.bits_per_sample / 8;
	std::uint32_t dst_stride = target.bits_per_sample / 8;
	std::size_t src_frame = src_stride * format_.channels;
	std::size_t dst_frame = dst_stride * target.channels;
	std::size_t num_frames = data_.size() / src_frame;

	octet_vector out(num_frames * dst_frame);

	for(std::size_t f = 0; f != num_frames; ++f) {
		auto* src = data_.data() + f * src_frame;
		auto* dst = out.data() + f * dst_frame;

		float channels[max_channels];
		for(std::uint32_t c = 0; c != format_.channels; ++c) {
			channels[c] = read_sample(src + c * src_stride, format_);
		}

		for(std::uint32_t c = 0; c != target.channels; ++c) {
			write_sample(dst + c * dst_stride, remap_channel(channels, format_.channels, target.channels, c), target);
		}
	}

	data_ = std::move(out);
	format_ = target;
}

void audio_data::load(std::string const& file, audio::audio_format format, file_format ff) {
	load(file, ff);
	if(format_ != format) {
		resample(format);
	}	
}

void audio_data::load(std::string const& file, file_format ff) {
	if(ff == auto_format && file_extension(file) == "wav") {
		ff = wav;
	}
	if(ff == wav) {
		std::ifstream in(file, std::ios_base::binary);
		if(!in) {
			throw std::runtime_error("failed to open file: " + file);
		}
		audio::wav w(data_);
		w.load(in);
		format_ = w.format();
	} else {
		throw invalid_format("format not supported");
	}
}

void audio_data::save(std::string const& file, file_format ff) {
	if(ff == auto_format && file_extension(file) == "wav") {
		ff = wav;
	}
	if(ff == wav) {
		std::ofstream out(file, std::ios_base::binary);
		if(!out) {
			throw std::runtime_error("failed to open file: " + file);
		}
		audio::wav w(data_);
		w.save(out, format_);
	} else {
		throw invalid_format("format not supported");
	}
}

void audio_data::save(std::string const& file, audio::audio_format const& format, file_format ff) {
	audio_data copy = *this;
	if(copy.format_ != format) {
		copy.resample(format);
	}
	copy.save(file, ff);
}

audio::audio_format audio_data::format() const {
	return format_;
}

octet_vector audio_data::data() const {
	return data_;
}

audio::audio_buffer load_file_to_buffer(std::string const& file, audio_data::file_format f) {
	audio_data pcm;
	pcm.load(file, f);
	return audio::audio_buffer(pcm.format(), pcm.data());
}

}

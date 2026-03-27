#include "wav.hpp"

#include <securepath/log/log.hpp>

#include <istream>
#include <cstring>

namespace securepath::audio {

using namespace securepath::audio::riff;

static octet_vector read(std::istream& in, std::size_t size) {
	octet_vector buf(size);
	in.read(reinterpret_cast<char*>(buf.data()), buf.size());
	return in ? buf : octet_vector();
}

void wav::save(std::ostream& out, audio::audio_format const& format) {

	if(format.endian != std::endian::little) {
		throw invalid_format("unsupported endian type");
	}
	if(format.type != audio::uchar_t && format.type != audio::short_t && format.type != audio::float_t) {
		throw invalid_format("wav only supports uchar_t, short_t and float_t sample types");
	}

	using namespace riff;
	riff_header header(std::size_t(riff_fmt::size)+std::size_t(riff_data::size)+data_.size());

	riff_fmt fmt;
	// 1 = PCM, 3 = IEEE float
	fmt.data.audio_format = format.type == audio::float_t ? 3 : 1;
	fmt.data.channels = format.channels;
	fmt.data.sample_rate = format.samples_per_second;
	fmt.data.byte_rate = format.samples_per_second*format.channels*format.bits_per_sample/8;
	fmt.data.block_align = format.channels*format.bits_per_sample/8;
	fmt.data.bits_per_sample = format.bits_per_sample;

	riff_data data(data_);

	octet_vector buf(std::size_t(riff_header::size)+std::size_t(riff_fmt::size)+std::size_t(riff_data::size)+data_.size());
	std::size_t p = header.write(buf.data(), buf.size());
	p += fmt.write(buf.data()+p, buf.size()-p);
	data.write(buf.data()+p, buf.size()-p);

	out.write(reinterpret_cast<char const*>(buf.data()), buf.size());
}

void wav::load(std::istream& in) {
	data_.clear();

	octet_vector buf = read(in, riff_header::size);
	header_.read(buf.data(), buf.size());

	if(std::strncmp(reinterpret_cast<char const*>(header_.header.chunk_id), "RIFF", 4) != 0) {
		throw invalid_format("not a RIFF file");
	}
	if(std::strncmp(reinterpret_cast<char const*>(header_.format), "WAVE", 4) != 0) {
		throw invalid_format("not a RIFF WAVE file");
	}

	for(;in;) {
		load_chunk(in);
	}

	if(!format_) {
		throw invalid_format("format chunk not found");
	}
	if(data_.empty()) {
		throw invalid_format("data chunk not found");
	}
	if(format_->bits_per_sample != 8 && format_->bits_per_sample != 16 && format_->bits_per_sample != 24 && format_->bits_per_sample != 32) {
		LOG_INFO("invalid bits_per_sample: {}", format_->bits_per_sample);
		throw invalid_format("only 8, 16, 24 or 32 bits per sample is supported");
	}
}

void wav::load_chunk(std::istream& in) {
	LOG_TRACE("reading chunk header at offset {}", std::size_t(in.tellg()));
	octet_vector buf = read(in, chunk_header::size);

	if(!buf.empty()) {
		chunk_header h;
		h.read(buf.data(), buf.size());
		LOG_INFO("found chunk header '{}' with size '{}'", std::string(h.chunk_id, h.chunk_id+4), h.chunk_size);
		if(std::strncmp(reinterpret_cast<char const*>(h.chunk_id), "fmt ", 4) == 0) {
			load_format_chunk(in, h.chunk_size);
		} else if(std::strncmp(reinterpret_cast<char const*>(h.chunk_id), "data", 4) == 0) {
			load_data_chunk(in, h.chunk_size);
		} else {
			LOG_TRACE("skipping RIFF chunk '{}' ({} bytes)", std::string(h.chunk_id, h.chunk_id+4), h.chunk_size);
			in.seekg(h.chunk_size, std::ios_base::cur);
		}
	}
}

void wav::load_format_chunk(std::istream& in, std::size_t size) {
	octet_vector buf = read(in, size);
	if(!buf.empty()) {
		riff_fmt_data fmt;
		fmt.read(buf.data(), buf.size());
		format_ = fmt;
	}
}

void wav::load_data_chunk(std::istream& in, std::size_t size) {
	data_ = read(in, size);
	if(data_.size() != size) {
		throw invalid_format("invalid data chunk; size does not match");
	}
}

audio::audio_format wav::format() const {
	if(!format_) {
		throw invalid_format("format chunk not found");
	}
	auto type = [this]() -> audio::sample_type {
		if(format_->audio_format == 3) {
			return audio::float_t;
		}
		if(format_->bits_per_sample == 8) {
			return audio::uchar_t;
		}
		if(format_->bits_per_sample == 24) {
			return audio::int24_t;
		}
		return audio::short_t;
	};

	return audio::audio_format
		{ type()
		, format_->channels
		, format_->bits_per_sample
		, format_->sample_rate
		, std::endian::little };
}

}

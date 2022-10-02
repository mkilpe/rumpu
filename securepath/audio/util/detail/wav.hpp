#pragma once

#include <securepath/audio/util/audio_data.hpp>
#include <securepath/audio/util/riff_format.hpp>

namespace securepath::audio {

class wav {
public:
	wav(octet_vector& d)
	: data_(d)
	{}

	void load(std::istream& in);
	void save(std::ostream& out, audio::audio_format const& format);

	audio::audio_format format() const;

private:
	void load_chunk(std::istream& in);
	void load_format_chunk(std::istream& in, std::size_t size);
	void load_data_chunk(std::istream& in, std::size_t size);

private:
	octet_vector& data_;
	riff::riff_header header_;
	std::optional<riff::riff_fmt_data> format_;
};

}


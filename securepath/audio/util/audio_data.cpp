#include "audio_data.hpp"
#include "detail/wav.hpp"

#include <fstream>

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

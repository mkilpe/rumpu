#ifndef SECUREPATH_OPUS_AUDIO_CODEC_HEADER
#define SECUREPATH_OPUS_AUDIO_CODEC_HEADER

#include "audio_codec.hpp"

#include <opus.h>

namespace securepath::audio {

class opus_play_codec : public play_codec {
public:
	opus_play_codec(codec_config ccfg);
	~opus_play_codec();

	std::size_t decode(uint8_t const* in, std::size_t in_size, audio_buffer& out, std::size_t out_size = 0) override;

private:
	codec_config ccfg_;
	OpusDecoder* decoder_;
};

class opus_capture_codec : public capture_codec {
public:
	opus_capture_codec(codec_config ccfg);
	~opus_capture_codec();

	std::size_t encode(audio_buffer& in, std::size_t samples, uint8_t* out, std::size_t out_size) override;

private:
	codec_config ccfg_;
	OpusEncoder* encoder_;
};

}

#endif

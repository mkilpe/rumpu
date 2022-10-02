
#include "opus_codec.hpp"

#include <securepath/log/log.hpp>

#include <cassert>

namespace securepath::audio {

opus_play_codec::opus_play_codec(codec_config ccfg)
	: ccfg_(std::move(ccfg))
	, decoder_()
{
	int err = 0;
	decoder_ = opus_decoder_create(ccfg_.sample_rate, ccfg_.channels, &err);
	if( err < 0 ) {
		LOG_INFO("failed to create opus decoder: '%' with sample_rate = %, channels = %", opus_strerror(err), ccfg_.sample_rate, ccfg_.channels);
		throw codec_operation_failed("failed to create opus decoder");
	}
}

opus_play_codec::~opus_play_codec() {
	if( decoder_ ) {
		opus_decoder_destroy( decoder_ );
	}
}

std::size_t opus_play_codec::decode(std::uint8_t const* in, std::size_t in_size, audio_buffer& out, std::size_t out_size) {
	assert( out.format().type == short_t );
	std::size_t bsize = out_size ? std::min<std::size_t>(out_size, out.free_samples()) : out.free_samples();
	int err = opus_decode(decoder_, in, in_size, out.free_begin<int16_t>(), bsize/ccfg_.channels, 0);
	if(err < 0) {
		LOG_INFO("opus decode error: %", opus_strerror(err));
	} else if(in) {
		out.conserve_samples(err);
	} else {
		LOG_INFO("packet loss decode: % (%)", err, in_size);
		out.conserve_samples(err);
	}
	return std::max( err, 0 );
}

opus_capture_codec::opus_capture_codec(codec_config ccfg)
	: ccfg_(std::move(ccfg))
	, encoder_()
{
	int err = 0;
	encoder_ = opus_encoder_create(ccfg_.sample_rate, ccfg_.channels, OPUS_APPLICATION_VOIP, &err);
	if( err < 0 ) {
		LOG_INFO("failed to create ops encoder: '%' with sample_rate = %, channels = %", opus_strerror(err), ccfg_.sample_rate, ccfg_.channels);
		throw codec_operation_failed("failed to create opus encoder");
	}
	err = opus_encoder_ctl( encoder_, OPUS_SET_BITRATE(ccfg_.bit_rate) );
	if( err < 0 ) {
		LOG_INFO("opus encoder error: %", opus_strerror(err));
		throw codec_operation_failed("failed to set opus encoder bit rate");
	}
}

opus_capture_codec::~opus_capture_codec() {
	if( encoder_ ) {
		opus_encoder_destroy( encoder_ );
	}
}

std::size_t opus_capture_codec::encode(audio_buffer& in, std::size_t samples, std::uint8_t* out, std::size_t out_size) {
	assert( in.format().type == short_t );
	assert( samples <= in.used_samples() );
	assert( out_size >= samples*2 );
	int err = opus_encode(encoder_, in.begin<int16_t>(), samples/ccfg_.channels, out, out_size);
	if (err < 0) {
		LOG_INFO("opus encode error: %", opus_strerror(err));
	} else {
		in.consume_samples(samples);
	}
	return std::max( err, 0 );
}

}

#ifndef SECUREPATH_SERIALISATION_CODEC_ASN_DER_BUFFERED_STREAM_HEADER
#define SECUREPATH_SERIALISATION_CODEC_ASN_DER_BUFFERED_STREAM_HEADER

#include <securepath/serialisation/types.hpp>
#include <securepath/util/types.hpp>

#include <cassert>
#include <cstring>
#include <string>

namespace securepath::serialisation {

template<typename Stream>
class buffered_stream {
public:
	using stream_type = Stream;

	buffered_stream(stream_type& s)
	: stream_(s)
	, look_ahead_pos_()
	, real_pos_()
	, peek_count_()
	{}

	std::uint8_t read() {
		std::uint8_t b;
		read(&b,1);
		return b;
	}

	std::size_t read(std::uint8_t* buf, std::size_t s) {
		std::size_t ps = read_from_buffer(buf, s);
		if(ps != s) {
			ps = read_from_stream(buf+ps, s-ps);
		}
		return ps;
	}

	std::size_t read_from_buffer(std::uint8_t* buf, std::size_t s) {
		std::size_t rs = 0;
		if(look_ahead_pos_ < buffer_.size()) {
			rs = std::min(s, buffer_.size()-look_ahead_pos_);
			std::memcpy(buf, buffer_.data()+look_ahead_pos_, rs);
			look_ahead_pos_ += rs;
			if(!peek_count_ && look_ahead_pos_ == buffer_.size()) {
				real_pos_ += look_ahead_pos_;
				look_ahead_pos_ = 0;
				buffer_.clear();
			}
		}
		return rs;
	}

	std::size_t read_from_stream(std::uint8_t* buf, std::size_t s) {
		// notice: this is broken if the stream/deserialiser doesn't get re-constructed after failure
		// as this might make a partial read and throw the data away
		if(!stream_.read(reinterpret_cast<char*>(buf), s)) {
			throw serialisation_error("end of stream");
		}
		if(peek_count_) {
			buffer_.insert(buffer_.end(), buf, buf+s);
			look_ahead_pos_ += s;
		} else {
			real_pos_ += s;
		}
		return s;
	}

	bool readable_bytes(std::size_t s) {
		std::size_t rs = 0;
		if(look_ahead_pos_ < buffer_.size()) {
			rs = std::min(s, buffer_.size()-look_ahead_pos_);
		}
		if(rs != s) {
			std::size_t more_required = s-rs;
			buffer_.resize(buffer_.size()+more_required);
			// notice: this is broken if the stream/deserialiser doesn't get re-constructed after failure
			// as this might make a partial read and throw the data away
			if(!stream_.read(reinterpret_cast<char*>(buffer_.data()+buffer_.size()-more_required), more_required)) {
				throw serialisation_error("end of stream");
			}
		}
		return s;
	}

	bool is_peeking() const {
		return peek_count_ > 0;
	}

	std::size_t start_peek() {
		++peek_count_;
		return look_ahead_pos_;
	}

	void end_peek(std::size_t pos, bool commit = false) {
		assert(peek_count_);
		--peek_count_;
		if(!commit) {
			look_ahead_pos_ = pos;
		}
	}

	std::uint64_t pos() const {
		return real_pos_ + look_ahead_pos_;
	}
private:
	stream_type& stream_;
	octet_vector buffer_;
	std::size_t look_ahead_pos_;
	std::uint64_t real_pos_;
	int peek_count_;
};

template<typename Stream>
class stream_peek_guard {
public:
	stream_peek_guard(Stream& s)
	: s_(s)
	, p_(s.start_peek())
	{}
	~stream_peek_guard() {
		s_.end_peek(p_);
	}
private:
	Stream& s_;
	std::size_t p_;
};

}

#endif

#ifndef SECUREPATH_AUDIO_FORMAT_HEADER
#define SECUREPATH_AUDIO_FORMAT_HEADER

#include <securepath/serialisation/sequence.hpp>
#include <securepath/serialisation/serialiser.hpp>
#include <securepath/serialisation/deserialiser.hpp>

#include <cstdint>
#include <cstddef>
#include <iosfwd>

namespace securepath::audio {

enum sample_type {
	char_t,
	uchar_t,
	short_t,
	float_t
};

inline
auto& serialise(serialisation::serialiser& s, sample_type const& v) {
	s & int(v);
	return s;
}

inline
auto& serialise(serialisation::deserialiser& s, sample_type& v) {
	int i = 0;
	s & i;
	v = sample_type(i);
	return s;
}

enum endian_type {
	little_endian,
	big_endian
};

struct audio_format {
	sample_type type = short_t;
	std::uint32_t channels = 1;
	std::uint32_t bits_per_sample = 16;
	std::uint32_t samples_per_second = 24000;
	int endian = little_endian;

	template<typename Ser>
	void serialise(Ser& s) {
		serialisation::sequence seq(s);
		seq & type & channels & bits_per_sample & samples_per_second & endian;
	}
};

std::ostream& operator<<(std::ostream&, audio_format const&);

inline bool operator==(audio_format a1, audio_format a2) {
	return a1.type == a2.type
		&& a1.channels == a2.channels
		&& a1.bits_per_sample == a2.bits_per_sample
		&& a1.endian == a2.endian;
}

struct device_config {
	device_config(audio_format f, std::size_t buf_size)
	: format(f)
	, buffer_size(buf_size)
	, period_size(buffer_size/2)
	{}

	virtual ~device_config() {}

	audio_format format;
	std::size_t buffer_size; //in samples
	std::size_t period_size; //in samples
};

std::ostream& operator<<(std::ostream&, device_config const&);

}

#endif

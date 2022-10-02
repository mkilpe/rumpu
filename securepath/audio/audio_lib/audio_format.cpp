#include "audio_format.hpp"

#include <ostream>

namespace securepath::audio {
namespace {

char const* sample_type_name[4] = {"char_t", "uchar_t" , "short_t", "float_t"};
char const* endian_type_name[2] = {"little", "big"};

}

std::ostream& operator<<(std::ostream& out, audio_format const& c) {
	//todo: check array boundaries
	return out << "[" << sample_type_name[c.type]
			<< "," << c.channels << "," << c.bits_per_sample
			<< "," << c.samples_per_second << "," << endian_type_name[c.endian] << "]";
}

std::ostream& operator<<(std::ostream& out, device_config const& c) {
	return out << "{" << c.buffer_size << "," <<  c.period_size << "," << c.format << "}";
}

}

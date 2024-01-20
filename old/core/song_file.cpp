#include "song_file.hpp"

#include <securepath/serialisation/util.hpp>

#include <fstream>

namespace securepath::drum {

std::string const file_tag{"spd"};
int const format_version{1};

song load_song_file(std::string const& file) {
	std::ifstream in(file, std::ios_base::binary);
	if(!in) {
		throw std::runtime_error("unable to open file");
	}

	char array[3] = {};
	if(!in.read(array, 3) || file_tag != std::string(array, array+3)) {
		throw std::runtime_error("invalid file format");
	}

	serialisation::asn_der_decoder dec(in);
	serialisation::deserialiser deser(dec);

	int version{};
	deser & version;
	if(version != format_version) {
		throw std::runtime_error("incompatible file version");
	}

	song s;
	deser & s;
	return s;
}

void save_song_file(std::string const& file, song const& s) {
	std::ofstream out(file, std::ios_base::binary | std::ios_base::trunc);
	out.write(file_tag.data(), file_tag.size());
	serialisation::asn_der_encoder enc(out);
	serialisation::serialiser ser(enc);
	ser & format_version & s;
}

}

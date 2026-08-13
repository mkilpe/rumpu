#include "song_file.hpp"

#include <securepath/serialisation/util.hpp>
#include "serialisation/serialisation.hpp"

#include <fstream>

namespace securepath::drum {

std::string const file_tag{"spd"};
// v1: original format; v2: song-level track_settings added (2026-08)
int const format_version{2};

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
	// Older versions load (decode handles absent fields); newer ones are rejected.
	if(version < 1 || version > format_version) {
		throw std::runtime_error("incompatible file version");
	}

	song s;
	deser & s;
	// files saved before track_settings existed migrate from the first section
	s.sync_track_settings();
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

#include "song_file.hpp"

#include <securepath/serialisation/util.hpp>
#include "serialisation/serialisation.hpp"

#include <cmath>
#include <filesystem>
#include <format>
#include <fstream>

namespace securepath::drum {

// Bounds mirror what the engine can represent: the per-bar timing walk clamps
// slide-driven tempo to [tempo::min_bpm, tempo::max_bpm] (bar_timing.hpp) and
// divides by both time-signature fields.
static void validate_time_signature(time_signature const& ts, char const* what) {
	if(ts.beats_in_bar() < 1 || ts.beats_in_bar() > 128
			|| ts.beat_type() < 1 || ts.beat_type() > 128) {
		throw std::runtime_error(std::format("invalid {} {}/{} in project file",
			what, ts.beats_in_bar(), ts.beat_type()));
	}
}

static void validate_tempo(tempo const& t, char const* what) {
	// range check also rejects NaN
	if(!(t.value >= tempo::min_bpm && t.value <= tempo::max_bpm)) {
		throw std::runtime_error(std::format("invalid {} {} in project file", what, t.value));
	}
}

// slide deltas feed straight into per-bar tempo and volume arithmetic, where a
// NaN would poison the bar-length calculation and the audio output
static void validate_slide_value(float value, char const* what) {
	if(!std::isfinite(value)) {
		throw std::runtime_error(std::format("invalid {} {} in project file", what, value));
	}
}

static void validate_section(section const& sec, std::size_t track_count, std::size_t instrument_count) {
	if(sec.length() < 1) {
		throw std::runtime_error("zero-length section in project file");
	}
	if(sec.tracks().size() != track_count) {
		throw std::runtime_error("sections have differing track counts in project file");
	}
	for(auto const& t : sec.tracks()) {
		if(t.instrument_index() >= instrument_count) {
			throw std::runtime_error(std::format(
				"track references instrument {} but only {} exist in project file",
				t.instrument_index(), instrument_count));
		}
		if(t.bars().size() != sec.length()) {
			throw std::runtime_error("track bar count does not match section length in project file");
		}
		for(auto const& [bar, slide] : t.volume_slides()) {
			validate_slide_value(slide.value, "volume slide");
		}
	}
	for(auto const& [bar, change] : sec.changes()) {
		if(change.timing_change) {
			validate_time_signature(*change.timing_change, "time signature change");
		}
		if(change.tempo_change) {
			validate_tempo(*change.tempo_change, "tempo change");
		}
		if(change.tempo_slide_change) {
			validate_slide_value(change.tempo_slide_change->value, "tempo slide change");
		}
	}
}

std::string project_relative_path(std::string const& path, std::filesystem::path const& base) {
	if(base.empty()) {
		return path;
	}
	std::error_code ec;
	auto rel = std::filesystem::relative(path, base, ec);
	if(ec || rel.empty()) {
		return path;
	}
	// forward slashes so a project saved on Windows still resolves elsewhere
	return rel.generic_string();
}

void validate_song(song const& s) {
	validate_time_signature(s.default_time_signature(), "time signature");
	validate_tempo(s.default_tempo(), "tempo");
	if(s.global_tempo_slide()) {
		validate_slide_value(s.global_tempo_slide()->value, "global tempo slide");
	}
	std::size_t const track_count = s.sections().empty()
		? 0 : s.sections().begin()->second.tracks().size();
	for(auto const& [id, sec] : s.sections()) {
		validate_section(sec, track_count, s.instruments().size());
	}
	for(auto id : s.section_order()) {
		if(!s.find_section(id)) {
			throw std::runtime_error(std::format(
				"section order references missing section {} in project file", id));
		}
	}
}

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
	validate_song(s);
	return s;
}

// Writes to a temporary file and renames over the target, so a failed save
// (disk full, permissions...) throws instead of silently truncating the
// previous good save.
void save_song_file(std::string const& file, song const& s) {
	std::string const tmp = file + ".tmp";
	try {
		std::ofstream out(tmp, std::ios_base::binary | std::ios_base::trunc);
		if(!out) {
			throw std::runtime_error("unable to open file for writing: " + tmp);
		}
		out.write(file_tag.data(), file_tag.size());
		serialisation::asn_der_encoder enc(out);
		serialisation::serialiser ser(enc);
		ser & format_version & s;
		out.flush();
		if(!out) {
			throw std::runtime_error("failed to write file: " + tmp);
		}
	} catch(...) {
		std::error_code ec;
		std::filesystem::remove(tmp, ec);
		throw;
	}
	std::error_code ec;
	std::filesystem::rename(tmp, file, ec);
	if(ec) {
		// mingw's std::filesystem::rename refuses to replace an existing
		// destination on Windows; remove it and retry. The temp file survives
		// a failure here, so the data is never lost.
		std::filesystem::remove(file, ec);
		std::filesystem::rename(tmp, file);
	}
}
}

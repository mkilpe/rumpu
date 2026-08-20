#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/instrument.hpp>

namespace securepath::drum::test {

inline char const* kick_path = TEST_DATA_DIR "/test_kick.wav";

// the shared fixture for mixer/player/export tests: a 4/4 song with one
// section and a kick hit on the first beat, instruments loaded
inline song make_kick_song(float bpm = 120, std::uint32_t sample_rate = 44100) {
	song s{{}, {4, 4}, {bpm}};
	s.add_instrument(instrument{kick_path});
	s.load_instruments(sample_rate);
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	beat b;
	b.action = beat::hit;
	b.hit_data.volume = volume{false, 1.0f};
	s.find_section(sec_id)->tracks()[0].bars()[0].beats[0] = b;
	return s;
}

}

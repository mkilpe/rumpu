#ifndef SPDRUM_COMMON_MIXER_HEADER
#define SPDRUM_COMMON_MIXER_HEADER

#include <rumpu/core/song.hpp>
#include <memory>

namespace securepath::drum {

class mixer {
public:
	//play whole song
	mixer(song const&, bool loop, std::uint32_t sample_rate);
	//play specific section only
	mixer(song const&, std::uint32_t section, bool loop, std::uint32_t sample_rate);
	~mixer();

	std::size_t process(float* buffer, std::size_t samples);

	std::uint32_t currently_playing_bar() const;
	std::uint32_t currently_playing_section() const;
	float play_position() const;
private:
	class impl;
	std::unique_ptr<impl> impl_;
};

}

#endif
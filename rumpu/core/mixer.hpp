#pragma once

#include <rumpu/core/song.hpp>
#include <memory>

namespace securepath::drum {

class mixer {
public:
	//play whole song
	mixer(song const&, std::uint32_t sample_rate);
	//play specific section only
	mixer(song const&, std::uint32_t section, std::uint32_t sample_rate);
	~mixer();

	std::size_t process(float* buffer, std::size_t samples);

	float play_position() const;
	float duration() const;
	std::uint32_t currently_playing_bar() const;
	float bar_progress() const;
	std::uint32_t samples_per_current_bar() const;
	std::uint32_t currently_playing_section() const;
	bool is_playing() const;
private:
	class impl;
	std::unique_ptr<impl> impl_;
};

}

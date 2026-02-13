#pragma once

#include <securepath/audio/audio_lib/audio_device.hpp>
#include <securepath/audio/audio_lib/util.hpp>
#include <securepath/event_system/event_handler.hpp>

#include "mixer.hpp"
#include "song.hpp"

#include <mutex>
#include <condition_variable>
#include <thread>

namespace securepath::drum {

namespace event {
	struct player_pos_changed {
    	typedef void type();
	};
}

class player {
public:
	player(event_system::event_handler& h, audio::device_config const& config = {{audio::float_t}, 16000});
	~player();

	void play(song const*, bool loop = false, std::uint32_t section = 0);
	void stop();

	audio::length_type current_play_time() const;
	int current_play_bar() const;

	void set_gain(float);
	float gain() const;

private:
	void play_entry();
	void write_data();

private:
	mutable std::mutex mutex_;
	std::condition_variable cond_;	
	event_system::event_handler& handler_;
	std::unique_ptr<audio::audio_play_device> out_;
	song const* song_;
	std::optional<mixer> mixer_;
	audio::audio_buffer buffer_;
	float gain_{1.0f};
	bool running_{true};
	std::jthread thread_;	
};

}

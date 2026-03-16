#pragma once

#include <securepath/audio/audio_lib/audio_device.hpp>
#include <securepath/audio/audio_lib/util.hpp>
#include <securepath/event_system/event_handler.hpp>

#include "mixer.hpp"
#include "song.hpp"

#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace securepath::drum {

struct play_status {
	bool playing{};
	std::chrono::milliseconds current_time{};
	std::chrono::milliseconds total_time{};
	std::uint32_t section_id{};
	std::uint32_t current_bar{};
	std::uint32_t total_bars{};
};

namespace event {
	struct player_pos_changed {
    	typedef void type();
	};
}

class player {
public:
	player(event_system::event_handler& h, audio::device_config const& config = {{audio::float_t, 1, 32, 24000}, 8000});
	~player();

	void play(song*, bool loop = false, std::uint32_t section = 0);
	void stop();

	std::uint32_t sample_rate() const;

	audio::length_type current_play_time() const;
	std::uint32_t current_play_bar() const;

	bool is_playing() const;
	std::uint32_t current_section_id() const;
	play_status get_status() const;

	void set_gain(float);
	float gain() const;

private:
	void internal_stop();
	void play_entry();
	void write_data();

private:
	mutable std::mutex mutex_;
	std::condition_variable cond_;
	event_system::event_handler& handler_;
	std::shared_ptr<audio::audio_play_device> out_;
	song* song_{};
	std::optional<mixer> mixer_;
	audio::audio_buffer buffer_;
	float gain_{1.0f};
	bool running_{true};
	std::jthread thread_;	
};

}

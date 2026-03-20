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
#include <vector>

namespace securepath::drum {

struct play_status {
	bool playing{};
	std::chrono::milliseconds current_time{};
	std::chrono::milliseconds total_time{};
	std::uint32_t section_id{};
	std::uint32_t current_bar{};
	std::uint32_t total_bars{};
	float bar_progress{};
	// section progress as fraction 0..1 (buffer-latency compensated)
	float section_progress{};
};

namespace event {
	struct player_pos_changed {
    	typedef void type();
	};
}

class player {
public:
	player(event_system::event_handler& h, audio::device_config const& config = {{audio::float_t, 1, 32, 24000}, 8000});
	// Test constructor: inject a pre-made audio device
	player(event_system::event_handler& h, std::shared_ptr<audio::audio_play_device> device);
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
	void update_bar_offsets(std::uint32_t section_id);
	void compute_cursor(play_status&, std::uint64_t bar_played) const;

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

	// Play cursor state
	std::uint64_t section_rendered_{};
	std::uint32_t cursor_section_id_{};
	std::uint32_t last_bar_{};
	std::uint32_t pending_section_id_{};
	std::vector<std::uint64_t> bar_offsets_;
	// Drain: mixer done but device still playing buffered audio
	bool draining_{};
	std::chrono::steady_clock::time_point drain_start_time_;
	std::uint32_t drain_start_buffered_{};
};

}

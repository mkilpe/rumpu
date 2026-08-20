
#include <catch2/catch_all.hpp>

#include "song_fixtures.hpp"

#include <rumpu/core/player.hpp>
#include <rumpu/core/mixer.hpp>
#include <rumpu/core/song.hpp>
#include <rumpu/core/instrument.hpp>
#include <securepath/audio/audio_lib/audio_device.hpp>
#include <securepath/audio/audio_lib/audio_device_modes.hpp>
#include <securepath/event_system/event_handler.hpp>
#include <securepath/event_system/event_loop.hpp>

#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace securepath;
using namespace securepath::drum;

// Minimal event loop that discards all events
class null_event_loop : public event_system::event_loop {
public:
	void emit(event_system::receiver, std::unique_ptr<event_system::event_base>) override {}
	void remove_receiver(event_system::receiver) override {}
	event_system::timer_handle start_timer(event_system::receiver, duration, bool) override { return {}; }
	void stop_timer(event_system::timer_handle) override {}
};

class null_event_handler : public event_system::event_handler {
public:
	null_event_handler(event_system::event_loop& loop) : event_handler(loop) {}
	~null_event_handler() { stop_handler(); }
	void handle_event(std::unique_ptr<event_system::event_base>) override {}
};

// Mock audio device with time-based sample consumption
class mock_play_device : public audio::audio_play_device {
public:
	mock_play_device(audio::device_config config, int speed_factor = 50)
	: config_(config)
	, buffer_size_(config.buffer_size)
	, speed_factor_(speed_factor)
	{
	}

	void start() override {
		running_ = true;
		play_start_ = clock::now();
		total_written_ = 0;
	}

	void stop(audio::stop_type) override {
		running_ = false;
	}

	audio::device_config config() const override { return config_; }
	std::size_t buffer_size() const override { return buffer_size_; }

	std::size_t avail() const override {
		auto consumed = consumed_now();
		auto written = total_written_.load();
		auto used = written > consumed ? written - consumed : 0;
		if(used > buffer_size_) {
			used = buffer_size_;
		}
		return buffer_size_ - used;
	}

	bool wait() override {
		std::this_thread::sleep_for(std::chrono::microseconds(200));
		return running_.load();
	}

	std::size_t write(audio::audio_buffer& b) override {
		auto samples = b.used_samples();
		auto a = avail();
		if(samples > a) {
			samples = a;
		}
		total_written_ += samples;
		b.consume_samples(samples);
		return samples;
	}

	int supported_modes() const override {
		return audio::audio_device_mode::notifications;
	}

	void set_mode(audio::mode const&) override {}

	std::size_t total_written() const { return total_written_.load(); }
	bool is_running() const { return running_.load(); }

private:
	std::size_t consumed_now() const {
		if(!running_) {
			return total_written_;
		}
		auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
			clock::now() - play_start_).count();
		std::size_t should = static_cast<std::size_t>(
			elapsed_us * config_.format.samples_per_second / 1000000) * speed_factor_;
		auto written = total_written_.load();
		return std::min(should, written);
	}

	using clock = std::chrono::steady_clock;
	audio::device_config config_;
	std::size_t buffer_size_;
	int speed_factor_;
	clock::time_point play_start_;
	std::atomic<std::size_t> total_written_{0};
	std::atomic<bool> running_{false};
};

TEST_CASE("bar_offsets match mixer sample counts", "[player]") {
	song s{{}, {4, 4}, {120}};
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);
	auto& sec = *s.find_section(sec_id);

	mixer m{s, sec_id, 44100};

	// Record samples per bar from mixer
	std::vector<std::uint32_t> mixer_bar_samples;
	std::vector<float> buf(88200);
	for(std::uint32_t bar = 0; bar < sec.length(); ++bar) {
		auto spb = m.samples_per_current_bar();
		mixer_bar_samples.push_back(spb);
		m.process(buf.data(), spb);
	}

	// Compute offsets same way as player::update_bar_offsets
	auto default_ts = s.default_time_signature();
	auto current_timing = default_ts;
	auto current_tempo = s.default_tempo();
	auto global_slide = s.global_tempo_slide();
	tempo_slide current_slide{};

	for(std::uint32_t bar = 0; bar < sec.length(); ++bar) {
		auto change = sec.find_change(bar);
		if(change) {
			if(change->timing_change) {
				current_timing = *change->timing_change;
			}
			if(change->tempo_slide_change) {
				current_slide = *change->tempo_slide_change;
			}
			if(change->tempo_change) {
				current_tempo = *change->tempo_change;
			} else {
				if(current_slide.is_active(bar)) {
					current_tempo.value += current_slide.bar_delta();
				}
				if(global_slide) {
					current_tempo.value += global_slide->value;
				}
				current_tempo.value = std::clamp(current_tempo.value, 1.0f, 9999.0f);
			}
		}
		double timing_multiplier = default_ts.beat_type() * current_timing.beats_in_bar()
			/ double(default_ts.beats_in_bar() * current_timing.beat_type());
		auto computed = static_cast<std::uint32_t>(
			60.0 * timing_multiplier * 44100 * current_timing.beats_in_bar() / current_tempo.value);
		CHECK(computed == mixer_bar_samples[bar]);
	}
}

TEST_CASE("mixer total samples matches bar_offsets total", "[player]") {
	song s{{}, {4, 4}, {120}};
	auto sec_id = s.add_section();
	s.section_order().push_back(sec_id);

	mixer m{s, sec_id, 44100};

	// Run mixer to completion
	std::uint64_t total_produced = 0;
	std::vector<float> buf(4096);
	std::size_t produced = 0;
	do {
		produced = m.process(buf.data(), buf.size());
		total_produced += produced;
	} while(produced != 0);

	// Compute expected total from bar_offsets logic
	std::uint32_t sr = 44100;
	auto default_ts = s.default_time_signature();
	auto current_timing = default_ts;
	auto current_tempo = s.default_tempo();
	std::uint64_t cumulative = 0;
	auto& sec = *s.find_section(sec_id);

	for(std::uint32_t bar = 0; bar < sec.length(); ++bar) {
		double timing_multiplier = default_ts.beat_type() * current_timing.beats_in_bar()
			/ double(default_ts.beats_in_bar() * current_timing.beat_type());
		auto samples = static_cast<std::uint32_t>(
			60.0 * timing_multiplier * sr * current_timing.beats_in_bar() / current_tempo.value);
		cumulative += samples;
	}

	// Mixer produces bar samples + tail (instrument sustain after last bar)
	// Total produced should be >= cumulative bar samples
	CHECK(total_produced >= cumulative);
	// But not wildly more (tail should be bounded by instrument sample length)
	CHECK(total_produced < cumulative + 44100);
}

TEST_CASE("player cursor is monotonic and reaches end", "[player]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	// Use 480 BPM for fast playback with a small buffer
	auto config = audio::device_config{{audio::float_t, 1, 32, 44100}, 4000};
	auto mock = std::make_shared<mock_play_device>(config, 100);

	song s = test::make_kick_song(480);
	auto sec_id = s.section_order()[0];

	player p{handler, mock};
	p.play(&s, false, sec_id);

	std::vector<float> progress_values;
	float max_progress = 0.0f;
	int backward_count = 0;

	// Poll cursor at high frequency until playback has started and finished
	bool done = false;
	for(int i = 0; i < 5000 && !done; ++i) {
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		auto status = p.get_status();
		done = !status.playing && !progress_values.empty();
		if(status.playing) {
			float progress = status.section_progress;
			progress_values.push_back(progress);
			if(progress > max_progress) {
				max_progress = progress;
			}
			if(progress_values.size() > 1 && progress < progress_values[progress_values.size() - 2] - 0.01f) {
				++backward_count;
			}
		}
	}

	INFO("Collected " << progress_values.size() << " cursor samples");
	INFO("Max progress: " << max_progress);

	// Should have collected some samples
	REQUIRE(progress_values.size() > 10);
	// Cursor should have reached near the end
	CHECK(max_progress > 0.90f);
	// Should be mostly monotonic (allow tiny float imprecision)
	CHECK(backward_count < 3);
}

TEST_CASE("player cursor does not get stuck before end", "[player]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	auto config = audio::device_config{{audio::float_t, 1, 32, 44100}, 4000};
	auto mock = std::make_shared<mock_play_device>(config, 100);

	song s = test::make_kick_song(480);
	auto sec_id = s.section_order()[0];

	player p{handler, mock};
	p.play(&s, false, sec_id);

	int max_stuck = 0;
	int current_stuck = 0;
	float last_progress = -1.0f;
	float stuck_at = 0.0f;

	bool playing = true;
	for(int i = 0; i < 5000 && playing; ++i) {
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		auto status = p.get_status();
		playing = p.is_playing() && status.playing;
		if(playing) {
			float progress = status.section_progress;
			// Only count stuck when cursor is not near the end (drain phase advances slowly)
			if(progress < 0.95f && std::abs(progress - last_progress) < 0.001f && last_progress > 0.0f) {
				++current_stuck;
				if(current_stuck > max_stuck) {
					max_stuck = current_stuck;
					stuck_at = progress;
				}
			} else {
				current_stuck = 0;
			}
			last_progress = progress;
		}
	}

	INFO("Max consecutive stuck samples: " << max_stuck << " at progress " << stuck_at);
	// Should not get stuck for extended periods before reaching the end
	CHECK(max_stuck < 50);
}

TEST_CASE("player cursor with real ALSA", "[.][player][alsa]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	song s = test::make_kick_song(120);
	auto sec_id = s.section_order()[0];

	// Use real ALSA device with default config (24000 Hz, buffer 8000)
	player p{handler};
	p.play(&s, false, sec_id);

	std::vector<float> progress_values;
	float last_progress = -1.0f;
	int max_stuck = 0;
	int current_stuck = 0;
	float stuck_at = 0.0f;
	int backward_count = 0;

	// Poll at ~120 Hz (like a typical UI frame rate)
	bool playing = p.is_playing();
	while(playing) {
		std::this_thread::sleep_for(std::chrono::milliseconds(8));
		auto status = p.get_status();
		playing = p.is_playing() && status.playing;
		if(playing) {
			float progress = status.section_progress;
			progress_values.push_back(progress);

			if(progress_values.size() > 1 && progress < progress_values[progress_values.size() - 2] - 0.01f) {
				++backward_count;
			}

			if(progress < 0.99f && std::abs(progress - last_progress) < 0.001f && last_progress > 0.0f) {
				++current_stuck;
				if(current_stuck > max_stuck) {
					max_stuck = current_stuck;
					stuck_at = progress;
				}
			} else {
				current_stuck = 0;
			}
			last_progress = progress;
		}
	}

	float max_progress = 0.0f;
	for(float v : progress_values) {
		if(v > max_progress) {
			max_progress = v;
		}
	}

	// Print cursor trajectory for debugging
	INFO("Collected " << progress_values.size() << " samples");
	INFO("Max progress: " << max_progress);
	INFO("Backward jumps: " << backward_count);
	INFO("Max stuck (before end): " << max_stuck << " at progress " << stuck_at);

	// Print last 20 values to see what happens near the end
	std::string last_values;
	std::size_t start = progress_values.size() > 20 ? progress_values.size() - 20 : 0;
	for(std::size_t i = start; i < progress_values.size(); ++i) {
		last_values += std::to_string(progress_values[i]) + " ";
	}
	INFO("Last 20 values: " << last_values);

	REQUIRE(progress_values.size() > 50);
	CHECK(max_progress > 0.95f);
	CHECK(backward_count < 3);
	// Should not be stuck for more than ~5 frames (40ms) before reaching end
	CHECK(max_stuck < 10);
}

TEST_CASE("player cursor resets when same section repeats", "[player]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	auto config = audio::device_config{{audio::float_t, 1, 32, 44100}, 4000};
	auto mock = std::make_shared<mock_play_device>(config, 100);

	// Song with same section twice in queue
	song s = test::make_kick_song(480);
	auto sec_id = s.section_order()[0];
	s.section_order().push_back(sec_id);

	player p{handler, mock};
	p.play(&s);

	int reset_count = 0;
	bool past_midpoint = false;

	bool playing = true;
	for(int i = 0; i < 10000 && playing; ++i) {
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		auto status = p.get_status();
		playing = p.is_playing() && status.playing;
		if(playing) {
			float progress = status.section_progress;
			if(progress > 0.5f) {
				past_midpoint = true;
			}
			// Detect cursor resetting to near zero after reaching past midpoint
			if(past_midpoint && progress < 0.1f) {
				++reset_count;
				past_midpoint = false;
			}
		}
	}

	INFO("Reset count: " << reset_count);
	CHECK(reset_count >= 1);
}

TEST_CASE("player cursor survives song edits during playback", "[player]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	auto config = audio::device_config{{audio::float_t, 1, 32, 44100}, 4000};
	auto mock = std::make_shared<mock_play_device>(config, 100);

	// repeat the section so the cursor recomputes bar offsets on every
	// transition while the song is being edited
	song s = test::make_kick_song(480);
	auto sec_id = s.section_order()[0];
	for(int i = 0; i < 30; ++i) {
		s.section_order().push_back(sec_id);
	}

	player p{handler, mock};
	p.play(&s);

	// storm of edits under the song's exclusive lock, mutating the structures
	// the cursor walks: the per-bar change map and the section map. checking
	// is_playing() rarely keeps the loop bounded without serialising every
	// edit against the audio thread through the player mutex.
	bool playing = true;
	for(int i = 0; i < 20000 && playing; ++i) {
		{
			std::unique_lock l{s.mutex};
			s.find_section(sec_id)->set_tempo_change(2, tempo{480});
		}
		{
			std::unique_lock l{s.mutex};
			s.find_section(sec_id)->set_tempo_change(2, std::nullopt);
		}
		{
			std::unique_lock l{s.mutex};
			auto id = s.add_section();
			s.remove_section(id);
		}
		if(i % 50 == 49) {
			playing = p.is_playing();
		}
	}

	// playback must have completed; the real assertion is a clean run under
	// the thread sanitizer
	for(int i = 0; i < 5000 && p.is_playing(); ++i) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	CHECK(!p.is_playing());
}

TEST_CASE("player cursor starts near zero", "[player]") {
	null_event_loop loop;
	null_event_handler handler{loop};

	auto config = audio::device_config{{audio::float_t, 1, 32, 44100}, 4000};
	auto mock = std::make_shared<mock_play_device>(config, 100);

	song s = test::make_kick_song(120);
	auto sec_id = s.section_order()[0];

	player p{handler, mock};
	p.play(&s, false, sec_id);

	// Give the player a moment to start
	std::this_thread::sleep_for(std::chrono::milliseconds(5));
	auto status = p.get_status();

	CHECK(status.playing);
	CHECK(status.section_progress < 0.1f);
	CHECK(status.current_bar == 0);
}


#include "player.hpp"
#include "bar_timing.hpp"
#include <securepath/audio/audio_lib/audio_device_modes.hpp>
#include <securepath/audio/audio_lib/audio_interface.hpp>
#include <securepath/audio/audio_lib/util.hpp>
#include <securepath/log/log.hpp>

#include <algorithm>
#include <stdexcept>

namespace securepath::drum {

namespace {

std::shared_ptr<audio::audio_play_device> open_default_play_device(audio::device_config const& config) {
	std::shared_ptr<audio::audio_play_device> out;
	auto interface = audio::create_default_audio_interface();
	if(interface) {
		out = interface->play_device(config);
	}
	if(!out) {
		throw std::runtime_error("Failed to find audio play device");
	}
	out->set_mode(audio::notification_mode{out->config().period_size});
	return out;
}

}

// out_ and buffer_ must be fully constructed before thread_ starts: a throw after
// thread_ is running would make ~jthread join a thread that never gets woken.
player::player(event_system::event_handler& h, audio::device_config const& config)
: handler_(h)
, out_(open_default_play_device(config))
, buffer_(out_->config().format, out_->config().buffer_size)
, thread_([&]{ play_entry(); })
{
}

player::player(event_system::event_handler& h, std::shared_ptr<audio::audio_play_device> device)
: handler_(h)
, out_(std::move(device))
, buffer_(out_->config().format, out_->config().buffer_size)
, thread_([&]{ play_entry(); })
{
}

player::~player() {
	{
		std::unique_lock l{mutex_};
		running_ = false;
		if(out_) {
			try {
				out_->stop();
			} catch(...)
			{}
		}
	}
	cond_.notify_one();
}

void player::update_bar_offsets(std::uint32_t section_id) {
	bar_offsets_.clear();
	if(!song_|| !out_) {
		return;
	}
	auto const* sec = song_->find_section(section_id);
	if(!sec) {
		return;
	}

	bar_timing timing{song_->default_time_signature(), song_->default_tempo(), song_->global_tempo_slide()};
	timing.new_section();

	std::uint64_t cumulative = 0;
	auto sr = out_->config().format.samples_per_second;
	bar_offsets_.reserve(sec->length() + 1);
	bar_offsets_.push_back(0);

	for(std::uint32_t bar = 0; bar < sec->length(); ++bar) {
		timing.begin_bar(*sec, bar);
		cumulative += timing.bar_samples(sr);
		bar_offsets_.push_back(cumulative);
	}
}

void player::play(song* s, bool, std::uint32_t section) {
	stop();
	std::unique_lock l{mutex_};
	draining_ = false;
	song_ = s;
	if(out_) {
		for(auto const& inst : song_->instruments()) {
			if(!inst.is_loaded()) {
				throw std::runtime_error("Instruments must be loaded before playing");
			}
		}
		auto sr = out_->config().format.samples_per_second;
		if(section) {
			mixer_.emplace(*song_, section, sr);
		} else {
			mixer_.emplace(*song_, sr);
		}

		section_rendered_ = 0;
		cursor_section_id_ = mixer_->currently_playing_section();
		last_bar_ = 0;
		pending_section_id_ = 0;
		update_bar_offsets(cursor_section_id_);

		out_->start();
		write_data();
		cond_.notify_one();
	}
}

std::uint32_t player::sample_rate() const {
	std::unique_lock l{mutex_};
	return out_ ? out_->config().format.samples_per_second : 0;
}

audio::length_type player::current_play_time() const {
	std::unique_lock l{mutex_};
	return mixer_ && out_
		? audio::length_type{uint32_t(mixer_->play_position()*1000)} - audio::samples_to_length(out_->config().format, out_->buffer_size() - out_->avail())
		: audio::length_type{};
}

std::uint32_t player::current_play_bar() const {
	std::unique_lock l{mutex_};
	return mixer_ ? mixer_->currently_playing_bar() : 0;
}

bool player::is_playing() const {
	std::unique_lock l{mutex_};
	return draining_ || mixer_.has_value();
}

std::uint32_t player::current_section_id() const {
	std::unique_lock l{mutex_};
	return mixer_ ? mixer_->currently_playing_section() : 0;
}

void player::compute_cursor(play_status& status, std::uint64_t bar_played) const {
	status.section_id = cursor_section_id_;
	if(song_) {
		if(auto const* sec = song_->find_section(cursor_section_id_)) {
			status.total_bars = sec->length();
		}
	}
	if(status.total_bars == 0 || bar_offsets_.size() < 2) {
		return;
	}
	std::uint64_t section_total = bar_offsets_.back();
	bar_played = std::min(bar_played, section_total);

	auto it = std::upper_bound(bar_offsets_.begin(), bar_offsets_.end(), bar_played);
	if(it != bar_offsets_.begin()) {
		--it;
	}
	std::uint32_t bar = static_cast<std::uint32_t>(it - bar_offsets_.begin());
	if(bar >= status.total_bars) {
		bar = status.total_bars - 1;
	}

	std::uint64_t bar_start = bar_offsets_[bar];
	std::uint64_t bar_len = bar_offsets_[bar + 1] - bar_start;
	float progress = bar_len > 0
		? static_cast<float>(bar_played - bar_start) / static_cast<float>(bar_len)
		: 0.0f;
	status.current_bar = bar;
	status.bar_progress = std::min(progress, 1.0f);
	status.section_progress = section_total > 0
		? std::min(1.0f, static_cast<float>(bar_played) / static_cast<float>(section_total))
		: 0.0f;
}

play_status player::get_status() const {
	std::unique_lock l{mutex_};
	play_status status;
	if(!mixer_ && !draining_) {
		return status;
	}

	status.playing = true;
	if(mixer_) {
		status.total_time = std::chrono::milliseconds{static_cast<long long>(mixer_->duration() * 1000)};
		auto pos = audio::length_type{uint32_t(mixer_->play_position()*1000)}
		         - audio::samples_to_length(out_->config().format, out_->buffer_size() - out_->avail());
		status.current_time = std::min(pos, status.total_time);
	}

	// Estimate samples still in device buffer: wall-clock during drain (ALSA
	// avail() jumps in period steps and returns buffer_size on XRUN), ALSA during play
	std::uint32_t buffered{};
	if(draining_) {
		auto elapsed = std::chrono::steady_clock::now() - drain_start_time_;
		auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
		auto drained = static_cast<std::uint64_t>(elapsed_us) * out_->config().format.samples_per_second / 1000000;
		buffered = drained < drain_start_buffered_
			? drain_start_buffered_ - static_cast<std::uint32_t>(drained) : 0;
	} else {
		buffered = out_->buffer_size() - out_->avail();
	}

	std::uint64_t section_total = bar_offsets_.empty() ? 0 : bar_offsets_.back();
	std::uint64_t bar_produced = std::min(section_rendered_, section_total);
	std::uint64_t tail = section_rendered_ > section_total ? section_rendered_ - section_total : 0;
	std::uint64_t bar_in_buf = buffered > tail ? buffered - tail : 0;
	std::uint64_t bar_played = bar_produced > bar_in_buf ? bar_produced - bar_in_buf : 0;

	compute_cursor(status, bar_played);
	return status;
}

void player::set_gain(float v) {
	std::unique_lock l{mutex_};
	gain_ = v;
}

float player::gain() const {
	std::unique_lock l{mutex_};
	return gain_;
}

void player::write_data() {
	std::size_t process = std::min<std::size_t>(out_->avail(), buffer_.free_samples());
	if(process) {
		float* start = &*buffer_.free_begin<float>();
		int samples = mixer_->process(start, process);
		if(samples) {
			section_rendered_ += samples;

			// Detect section transition (includes same section replayed: bar resets)
			std::uint32_t bar_after = mixer_->currently_playing_bar();
			std::uint32_t section_after = mixer_->currently_playing_section();
			if(pending_section_id_ == 0) {
				bool transition = (section_after != cursor_section_id_
					|| (bar_after < last_bar_ && last_bar_ > 0)) && mixer_->is_playing();
				if(transition) {
					pending_section_id_ = section_after;
				}
			}
			// Commit deferred transition once old section audio has drained from buffer
			if(pending_section_id_ != 0) {
				std::uint64_t section_total = bar_offsets_.empty() ? 0 : bar_offsets_.back();
				if(section_rendered_ >= section_total + out_->buffer_size()) {
					section_rendered_ -= section_total;
					cursor_section_id_ = pending_section_id_;
					update_bar_offsets(cursor_section_id_);
					pending_section_id_ = 0;
				}
			}
			last_bar_ = bar_after;

			buffer_.conserve_samples(samples);
			std::for_each(start, start+samples, [&](float& v)
			{
				v *= gain_;
			});
			size_t res = out_->write(buffer_);
			if(res != static_cast<size_t>(samples)) {
				LOG_WARN("could not write all to buffer");
			}
			// Record buffer state after write; used as drain reference if mixer
			// finishes on the next call (ALSA may XRUN before we detect it)
			drain_start_time_ = std::chrono::steady_clock::now();
			drain_start_buffered_ = out_->buffer_size() - out_->avail();
		} else {
			draining_ = true;
			mixer_ = std::nullopt;
			buffer_.set_used_samples(0);
		}
		handler_.emit<event::player_pos_changed>();
	}
}

void player::play_entry() {
	std::unique_lock l{mutex_};
	while(running_) {
		if(mixer_ && out_) {
			auto out = out_;
			l.unlock();
			out->wait();
			l.lock();
			if(running_ && out_ && mixer_) {
				try {
					write_data();
				} catch(const std::exception& ex) {
					LOG_WARN("exception: {}", ex.what());
				}
			}
		} else if(draining_ && out_) {
			// Use wall-clock countdown from drain start instead of ALSA avail(),
			// which jumps to buffer_size on XRUN before audio actually finishes
			auto elapsed = std::chrono::steady_clock::now() - drain_start_time_;
			auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
			auto sr = out_->config().format.samples_per_second;
			auto consumed = static_cast<std::uint64_t>(elapsed_us) * sr / 1000000;
			if(consumed >= drain_start_buffered_) {
				draining_ = false;
				out_->stop();
				handler_.emit<event::player_pos_changed>();
			} else {
				handler_.emit<event::player_pos_changed>();
				cond_.wait_for(l, std::chrono::milliseconds(10));
			}
		} else {
			cond_.wait(l);
		}
	}
}

void player::stop() {
	std::unique_lock l{mutex_};
	draining_ = false;
	internal_stop();
}

void player::internal_stop() {
	if(out_) {
		out_->stop();
		mixer_ = std::nullopt;
		buffer_.set_used_samples(0);
		handler_.emit<event::player_pos_changed>();
	}
}

}

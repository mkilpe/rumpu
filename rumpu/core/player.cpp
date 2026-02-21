
#include "player.hpp"
#include <securepath/audio/audio_lib/audio_device_modes.hpp>
#include <securepath/audio/audio_lib/audio_interface.hpp>
#include <securepath/audio/audio_lib/util.hpp>
#include <securepath/log/log.hpp>

#include <algorithm>
#include <stdexcept>

namespace securepath::drum {

player::player(event_system::event_handler& h, audio::device_config const& config)
: handler_(h)
, thread_([&]{ play_entry(); })
{
	auto interface = audio::create_default_audio_interface();
	if(interface) {
		out_ = interface->play_device(config);
	}
	if(!out_) {
		throw std::runtime_error("Failed to find audio play device");
	}

	auto conf = out_->config();
	out_->set_mode(audio::notification_mode{conf.period_size});
	buffer_ = audio::audio_buffer(conf.format, conf.buffer_size);
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

void player::play(song const* s, bool, std::uint32_t section) {
	stop();
	std::unique_lock l{mutex_};
	song_ = s;
	if(out_) {
		auto sample_rate = out_->config().format.samples_per_second;
		// ensure instrument samples are loaded at the correct sample rate
		const_cast<song*>(song_)->load_instruments(sample_rate);
		if(section) {
			mixer_.emplace(*song_, section, sample_rate);
		} else {
			mixer_.emplace(*song_, sample_rate);
		}
		out_->start();
		write_data();
		cond_.notify_one();
	}
}

audio::length_type player::current_play_time() const {
	std::unique_lock l{mutex_};
	return mixer_ && out_
		? audio::length_type{uint32_t(mixer_->play_position()*1000)} - audio::samples_to_length(out_->config().format, out_->buffer_size() - out_->avail())
		: audio::length_type{};
}

int player::current_play_bar() const {
	std::unique_lock l{mutex_};
	//return mixer_ ? mixer_->currently_playing_bar() : 0;
	return 0;
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
			buffer_.conserve_samples(samples);
			std::for_each(start, start+samples, [&](float& v)
			{
				v *= gain_;
			});
			size_t res = out_->write(buffer_);
			if(res != static_cast<size_t>(samples)) {
				LOG_WARN("could not write all to buffer");
			}
		} else {
			internal_stop();
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
		} else {
			cond_.wait(l);
		}
	}
}

void player::stop() {
	std::unique_lock l{mutex_};
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
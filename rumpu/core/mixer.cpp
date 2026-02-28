
#include "mixer.hpp"

#include <securepath/log/log.hpp>
#include <securepath/util/timer.hpp>

#include <algorithm>
#include <cstring>

namespace securepath::drum {
namespace {
struct current_pos {
	// samples per second we use for the play
	std::uint32_t const sample_rate{};
	// position in bars in the section
	std::uint32_t bar_pos{};
	// current sample playing position in the bar
	std::uint32_t sample_pos{};
	// amount of samples for current bar
	std::uint32_t samples{};
	// position as seconds from the beginning
	float time_pos{};
	// id of currently playing section
	std::uint32_t section_id{};

	time_signature current_timing;
	tempo current_tempo;

	// global tempo slide per bar
	std::optional<delta_tempo> global_tempo_slide;
	tempo_slide current_tempo_slide;

	void new_section() {
		bar_pos = 0;
		current_tempo_slide = tempo_slide{};
	}

	void handle_change(section_bar_change const& change) {
		if(change.timing_change) {
			current_timing = *change.timing_change;
		}
		if(change.tempo_slide_change) {
			current_tempo_slide = *change.tempo_slide_change;
		}
		if(change.tempo_change) {
			current_tempo = *change.tempo_change;
		} else {
			if(current_tempo_slide.is_active(bar_pos)) {
				current_tempo.value += current_tempo_slide.bar_delta();
			}
			if(global_tempo_slide) {
				current_tempo.value += global_tempo_slide->value;
			}
			current_tempo.value = std::clamp(current_tempo.value, 1.0f, 9999.0f);
		}
	}

	void update_samples_to_play(time_signature const& default_timing) {
		double timing_multiplier = default_timing.beat_type()*current_timing.beats_in_bar()
				/ double(default_timing.beats_in_bar()*current_timing.beat_type());

		sample_pos = 0;
		samples = 60*timing_multiplier*sample_rate*current_timing.beats_in_bar() / current_tempo.value;
	}
};

struct action_data {
	std::uint32_t sample_pos{};
	beat::action_type action{beat::none};
	float volume{1.0f};
	std::optional<audio_falloff> falloff;
};

struct track_info {
	sample_buffer current_audio;
	// position of the current_audio in samples
	std::uint32_t audio_pos{};
	// volume of the current_audio
	float current_audio_volume{1.0f};
	// fall-off for current audio
	std::optional<audio_falloff> falloff;
	// track volume that takes into account the volume slide
	drum::volume volume{};
	// current volume slide for the track
	volume_slide current_volume_slide;
	// actions for current bar playing
	std::deque<action_data> actions;
};
}

struct mixer::impl {
	struct init {};
	impl(init, song const& s, std::uint32_t sample_rate)
	: song_(&s)
	, pos_{sample_rate}
	, infos_(song_->instruments().size())
	{
		pos_.global_tempo_slide = song_->global_tempo_slide();
		pos_.current_timing = song_->default_time_signature();
		pos_.current_tempo = song_->default_tempo();
		for(std::size_t i = 0; i != infos_.size(); ++i) {
			infos_[i].volume = song_->instruments()[i].volume();
		}
	}

	impl(song const& s, std::uint32_t sample_rate)
	: impl(init{}, s, sample_rate)
	{
		sec_order_ = song_->section_order();
		std::reverse(sec_order_.begin(), sec_order_.end());
		next_section();
		if(section_) {
			update_audio_params();
		}
	}

	impl(song const& s, std::uint32_t section, std::uint32_t sample_rate)
	: impl(init{}, s, sample_rate)
	{
		section_play_ = true;
		section_ = song_->find_section(section);
		pos_.section_id = section;
		if(section_) {
			update_audio_params();
		}
	}

	void set_next_section() {
		auto sec = sec_order_.back();
		section_ = song_->find_section(sec);
		pos_.section_id = sec;
		sec_order_.pop_back();
	}

	void next_section() {
		if(!sec_order_.empty()) {
			set_next_section();
			pos_.new_section();
		} else {
			section_ = nullptr;
			/*if(loop_) {
				pos_.time_pos = 0.0f;
				if(!section_play_) {
					sec_order_ = song_->section_order();
					std::reverse(sec_order_.begin(), sec_order_.end());
					set_next_section();
				}
				pos_.new_section();
			}*/
		}
	}

	bool is_playing() const {
		bool ret = section_;
		if(!ret) {
			auto it = infos_.begin();
			for(; it != infos_.end() && !it->current_audio; ++it) {}
			ret = it != infos_.end();
		}
		return ret;
	}

	void push_info_action(track_info& info, std::int32_t sample_pos, beat const& b) {
		if(b.action == beat::hit) {
			std::int32_t adj_offset = sample_pos + pos_.samples*b.hit_data.rand_hit_offset;
			// don't allow negative offsets here, those are handled by looking up the next bar afterwards
			if(adj_offset >= 0) {
				if(std::uint32_t(adj_offset) >= pos_.samples) {
					adj_offset = pos_.samples-1;
				}
				info.actions.push_back(
					action_data{static_cast<std::uint32_t>(adj_offset), b.action, calc_action_volume(info, b)});
			}
		} else if(b.action == beat::stop) {
			info.actions.push_back(action_data{0, b.action, 0.0f, b.stop_data.falloff});
		}
	}

	float calc_action_volume(track_info const& info, beat const& b) {
		return info.volume.value*b.combined_hit_volume();
	}

	void update_track_volume(track const& t, track_info& info) {
		if(pos_.bar_pos == 0) {
			auto vol_slide = t.find_volume_slide(pos_.bar_pos);
			info.current_volume_slide = vol_slide.value_or(volume_slide{});
		}
		if(info.current_volume_slide.is_active(pos_.bar_pos)) {
			info.volume.value += info.current_volume_slide.bar_delta();
		}
	}

	void update_track_info(track const& t, track_info& info) {
		assert(info.actions.empty());
		update_track_volume(t, info);
		bar const& b = t.bars()[pos_.bar_pos];
		if(!b.beats.empty()) {
			std::size_t samples_per_beat = pos_.samples / b.beats.size();
			std::int32_t sample_pos = 0;

			for(auto const& v : b.beats) {
				if(!v.division.empty()) {
					std::int32_t offset = sample_pos;
					std::size_t samples_per_div = samples_per_beat / v.division.size();
					for(auto const& div : v.division) {
						push_info_action(info, offset, div);
						offset += samples_per_div;
					}
				} else {
					push_info_action(info, sample_pos, v);
				}
				sample_pos += samples_per_beat;
			}
		}
		lookup_negative_beat_offset(t, info);
	}

	void lookup_negative_beat_offset(track const& t, track_info& info) {
		if(pos_.bar_pos+1 < section_->length()) {
			bar const& b = t.bars()[pos_.bar_pos+1];
			if(!b.beats.empty()) {
				beat const* value{};
				if(!b.beats.front().division.empty()) {
					value = &b.beats.front().division.front();
				} else {
					value = &b.beats.front();
				}
				if(value && value->action == beat::hit && value->hit_data.rand_hit_offset < 0) {
					std::int32_t adj_offset = pos_.samples + pos_.samples*value->hit_data.rand_hit_offset;
					info.actions.push_back(
						action_data{static_cast<std::uint32_t>(adj_offset), value->action, calc_action_volume(info, *value)});
				}
			}
		}
 	}

	void update_audio_params() {
		auto change = section_->find_change(pos_.bar_pos);
		if(change) {
			pos_.handle_change(*change);
		}
		pos_.update_samples_to_play(song_->default_time_signature());
	}

	void update_pos_data(std::uint32_t samples) {
		pos_.time_pos += samples/float(pos_.sample_rate);
		if(section_) {
			if(pos_.sample_pos + samples >= pos_.samples) {
				++pos_.bar_pos;
				if(pos_.bar_pos == section_->length()) {
					next_section();
				}
				if(section_) {
					update_audio_params();
				}
			} else {
				pos_.sample_pos += samples;
			}
		}
	}

	void process_track(float* buffer, std::size_t index, std::size_t samples) {
		track_info& info = infos_[index];
		if(!info.volume.mute) {
			if(section_ && pos_.sample_pos == 0) {
				update_track_info(section_->tracks()[index], info);
			}
			std::size_t processed{};
			while(processed < samples) {
				action_data const* d{};
				if(!info.actions.empty()) {
					d = &info.actions.front();
				}
				std::size_t end = d ? d->sample_pos : pos_.sample_pos + samples;
				if(pos_.sample_pos + processed < end) {
					// how many samples we have until the action or till end of requested amount
					std::size_t s = std::min(samples-processed, end-pos_.sample_pos-processed);
					fill_audio_buffer(buffer, info, s);
					processed += s;
					buffer += s;
				}
				if(d && processed < samples) {
					set_current_track_audio(index, info, *d);
					info.actions.pop_front();
				}
			}
		}
	}

	void set_current_track_audio(std::size_t index, track_info& info, action_data const& data) {
		assert(data.action != beat::none);
		if(data.action == beat::hit) {
			info.audio_pos = 0;
			info.falloff = std::nullopt;
			auto& instrument = song_->instruments()[index];
			if(instrument.is_valid()) {
				info.current_audio = instrument.sample_to_play().buffer();
			} else {
				info.current_audio = {};
			}			
			info.current_audio_volume = data.volume;
		} else if(info.current_audio) {
			info.falloff = data.falloff;
		}
	}

	void fill_audio_buffer(float* buffer, track_info& info, std::size_t samples) {
		if(info.current_audio) {
			std::size_t audio_samples_left = info.current_audio->size() - info.audio_pos;
			std::size_t process = std::min(samples, audio_samples_left);
			float const* audio = info.current_audio->data() + info.audio_pos;
			std::for_each(buffer, buffer+process,
				[&](float& v) {
					float value = *(audio++)*info.current_audio_volume;
					if(info.falloff) {
						value *= info.falloff->factor(1.0f/pos_.sample_rate);
					}
					v += value;
				});
			if(process == audio_samples_left || (info.falloff && info.falloff->is_done()) ) {
				info.current_audio = nullptr;
			} else {
				info.audio_pos += process;
			}
		}
	}

	song const* song_{};
	section const* section_{};
	bool loop_{};
	bool section_play_{};
	song::section_order_type sec_order_;
	current_pos pos_;
	std::vector<track_info> infos_;
};

mixer::mixer(song const& s, std::uint32_t sample_rate)
: impl_(std::make_unique<impl>(s, sample_rate))
{
}

mixer::mixer(song const& s, std::uint32_t section, std::uint32_t sample_rate)
: impl_(std::make_unique<impl>(s, section, sample_rate))
{
}

mixer::~mixer()
{
}

std::size_t mixer::process(float* buffer, std::size_t samples)  {
	std::size_t ret{};

	timer t;

	if(impl_->is_playing()) {
		std::fill(buffer, buffer+samples, 0.0f);

		while(ret < samples && impl_->is_playing()) {
			std::shared_lock l{impl_->song_->mutex};
			std::uint32_t samples_to_process = std::min<std::uint32_t>(impl_->pos_.samples-impl_->pos_.sample_pos, samples-ret);
			for(std::size_t i = 0; i != impl_->infos_.size(); ++i) {
				impl_->process_track(buffer+ret, i, samples_to_process);
			}
			impl_->update_pos_data(samples_to_process);
			ret += samples_to_process;
		}
	}

	LOG_TRACE("rendering {} samples took {} microseconds", ret, t.elapsed_microseconds());

	return ret;
}
std::uint32_t mixer::currently_playing_bar() const {
	return impl_->pos_.bar_pos;
}

std::uint32_t mixer::currently_playing_section() const {
	return impl_->pos_.section_id;
}

bool mixer::is_playing() const {
	return impl_->is_playing();
}

float mixer::play_position() const {
	return impl_->pos_.time_pos;
}

}

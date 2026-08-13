
#include "song.hpp"

namespace securepath::drum {

song::song(song_metainfo info, time_signature ts, tempo t)
: info_(std::move(info))
, default_time_signature_(ts)
, default_tempo_(t)
{
}

song::song(song const& o)
: info_(o.info_)
, default_time_signature_(o.default_time_signature_)
, default_tempo_(o.default_tempo_)
, instruments_(o.instruments_)
, sections_(o.sections_)
, section_order_(o.section_order_)
, accent_info_(o.accent_info_)
, rand_offset_(o.rand_offset_)
, rand_volume_(o.rand_volume_)
, tempo_slide_(o.tempo_slide_)
{
}

song& song::operator=(song const& o) {
	if (this != &o) {
		info_ = o.info_;
		default_time_signature_ = o.default_time_signature_;
		default_tempo_ = o.default_tempo_;
		instruments_ = o.instruments_;
		sections_ = o.sections_;
		section_order_ = o.section_order_;
		accent_info_ = o.accent_info_;
		rand_offset_ = o.rand_offset_;
		rand_volume_ = o.rand_volume_;
		tempo_slide_ = o.tempo_slide_;
	}
	return *this;
}

song::song(song&& o) noexcept
: info_(std::move(o.info_))
, default_time_signature_(o.default_time_signature_)
, default_tempo_(o.default_tempo_)
, instruments_(std::move(o.instruments_))
, sections_(std::move(o.sections_))
, section_order_(std::move(o.section_order_))
, accent_info_(std::move(o.accent_info_))
, rand_offset_(std::move(o.rand_offset_))
, rand_volume_(std::move(o.rand_volume_))
, tempo_slide_(std::move(o.tempo_slide_))
{
}

song& song::operator=(song&& o) noexcept {
	info_ = std::move(o.info_);
	default_time_signature_ = o.default_time_signature_;
	default_tempo_ = o.default_tempo_;
	instruments_ = std::move(o.instruments_);
	sections_ = std::move(o.sections_);
	section_order_ = std::move(o.section_order_);
	accent_info_ = std::move(o.accent_info_);
	rand_offset_ = std::move(o.rand_offset_);
	rand_volume_ = std::move(o.rand_volume_);
	tempo_slide_ = std::move(o.tempo_slide_);
	return *this;
}

time_signature song::default_time_signature() const {
	return default_time_signature_;
}

tempo song::default_tempo() const {
	return default_tempo_;
}

std::optional<delta_tempo> song::global_tempo_slide() const {
	return tempo_slide_;
}

volume_accent_info const& song::accent_rules() const {
	return accent_info_;
}

song_metainfo const& song::meta_info() const {
	return info_;
}

void song::set_metainfo(song_metainfo info) {
	info_ = std::move(info);
}

void song::set_default_time_signature(time_signature ts) {
	default_time_signature_ = ts;
}

void song::set_default_tempo(tempo t) {
	default_tempo_ = t;
}

rand_hit_offset const& song::rand_offset() const {
	return rand_offset_;
}

rand_hit_volume const& song::rand_volume() const {
	return rand_volume_;
}

void song::set_rand_offset(rand_hit_offset v) {
	rand_offset_ = v;
	randomise_beats();
}

void song::set_rand_volume(rand_hit_volume v) {
	rand_volume_ = v;
	randomise_beats();
}

void song::randomise_beat(beat& b) {
	if(b.action == beat::hit) {
		if(rand_offset_.max_ms > 0) {
			std::uniform_real_distribution<float> dist(-rand_offset_.max_ms, rand_offset_.max_ms);
			b.hit_data.rand_hit_offset = dist(rng_);
		} else {
			b.hit_data.rand_hit_offset = 0;
		}
		if(rand_volume_.max_percent > 0) {
			std::uniform_real_distribution<float> dist(-rand_volume_.max_percent, rand_volume_.max_percent);
			b.hit_data.rand_volume.value = dist(rng_) / 100.0f;
		} else {
			b.hit_data.rand_volume.value = 0;
		}
	}
	for(auto& div : b.division) {
		randomise_beat(div);
	}
}

void song::randomise_beats() {
	for(auto& [id, sec] : sections_) {
		for(auto& t : sec.tracks()) {
			for(auto& bar : t.bars()) {
				for(auto& b : bar.beats) {
					randomise_beat(b);
				}
			}
		}
	}
}

std::size_t song::add_instrument(instrument inst) {
	std::size_t idx = instruments_.size();
	instruments_.push_back(std::move(inst));
	return idx;
}

void song::remove_instrument(std::size_t index) {
	if(index >= instruments_.size()) {
		return;
	}
	instruments_.erase(instruments_.begin() + index);
	for(auto& [id, sec] : sections_) {
		auto& tracks = sec.tracks();
		std::erase_if(tracks, [index](track const& t) {
			return t.instrument_index() == index;
		});
		for(auto& t : tracks)
			if(t.instrument_index() > index)
				t.set_instrument_index(t.instrument_index() - 1);
	}
}

void song::load_instruments(std::uint32_t sample_rate, std::filesystem::path const& base_dir) {
	for(auto& i : instruments_) {
		i.load_samples(sample_rate, base_dir);
	}
}

void song::populate_default_beats(section& sec) {
	for(auto& t : sec.tracks()) {
		for(auto& b : t.bars()) {
			b.beats.resize(default_time_signature_.beats_in_bar());
		}
	}
}

std::uint32_t song::add_section(std::optional<section> s) {
	// size()+1 would reuse a live id after a removal and overwrite that section
	std::uint32_t index = sections_.empty() ? 1 : sections_.rbegin()->first + 1;
	if(s) {
		sections_[index] = std::move(*s);
	} else if(!sections_.empty()) {
		// Copy track layout from existing section so all sections stay consistent
		auto const& ref = sections_.begin()->second;
		sections_[index] = section{ref.length()};
		for(auto const& t : ref.tracks()) {
			auto& nt = sections_[index].add_track(t.instrument_index());
			nt.set_name(t.name());
		}
		populate_default_beats(sections_[index]);
	} else {
		sections_[index] = section{4, static_cast<std::uint32_t>(instruments_.size())};
		populate_default_beats(sections_[index]);
	}
	if(sections_[index].name().empty()) {
		sections_[index].set_name("Section " + std::to_string(index));
	}
	return index;
}

void song::add_section(std::uint32_t id) {
	sections_[id] = section{4, static_cast<std::uint32_t>(instruments_.size())};
	if(sections_[id].name().empty()) {
		sections_[id].set_name("Section " + std::to_string(id));
	}
	populate_default_beats(sections_[id]);
}

void song::remove_section(std::uint32_t id) {
	sections_.erase(id);
	std::erase(section_order_, id);
}

}

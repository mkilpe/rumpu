
#include "song.hpp"

namespace securepath::drum {

song::song(song_metainfo info, time_signature ts, tempo t)
: info_(std::move(info))
, default_time_signature_(ts)
, default_tempo_(t)
{
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

void song::load_instruments(std::uint32_t sample_rate) {
	for(auto& i : instruments_) {
		i.load_samples(sample_rate);
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
	std::uint32_t index = sections_.size()+1;
	if(s) {
		sections_[index] = std::move(*s);
		if(sections_[index].name().empty()) {
			sections_[index].set_name("Section " + std::to_string(index));
		}
	} else {
		//t: take the default length from settings or from previous section?
		sections_[index] = section{4, static_cast<std::uint32_t>(instruments_.size())};
		if(sections_[index].name().empty()) {
			sections_[index].set_name("Section " + std::to_string(index));
		}
		populate_default_beats(sections_[index]);
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

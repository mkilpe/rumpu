
#include "song.hpp"

namespace securepath::drum {

song::song(song_metainfo info, time_signature ts, tempo t)
: info_(std::move(info))
, default_time_signature_(ts)
, default_tempo_(t)
{
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

song::sections_type& song::sections() {
	return sections_;
}

song::section_order_type const& song::section_order() const {
	return section_order_;
}

song::section_order_type& song::section_order() {
	return section_order_;
}

std::vector<instrument> const& song::instruments() const {
	return instruments_;
}

std::vector<instrument>& song::instruments() {
	return instruments_;
}

volume_accent_info const& song::accent_rules() const {
	return accent_info_;
}

song_metainfo const& song::meta_info() const {
	return info_;
}

void song::add_instrument(instrument inst) {
	instruments_.push_back(std::move(inst));
	for(auto& t : sections_) {
		t.second.add_track();
	}
}

void song::load_instruments(std::uint32_t sample_rate) {
	for(auto& i : instruments_) {
		i.load_samples(sample_rate);
	}
}

section const* song::find_section(std::uint32_t id) const {
	auto it = sections_.find(id);
	return it != sections_.end() ? &it->second : nullptr;
}

section* song::find_section(std::uint32_t id) {
	auto it = sections_.find(id);
	return it != sections_.end() ? &it->second : nullptr;
}

std::uint32_t song::add_section(std::optional<section> s) {
	std::uint32_t index = sections_.size()+1;
	if(s) {
		sections_[index] = std::move(*s);
	} else {
		//t: take the default length from settings or from previous section?
		sections_[index] = section{4, static_cast<std::uint32_t>(instruments_.size())};
	}
	return index;
}

void song::add_section(std::uint32_t id) {
	sections_[id] = section{4, static_cast<std::uint32_t>(instruments_.size())};
}

}

#pragma once

#include <securepath/serialisation/enum.hpp>
#include <securepath/serialisation/vector.hpp>
#include <securepath/serialisation/sequence.hpp>
#include <securepath/serialisation/tag.hpp>

#include "../bar.hpp"
#include "../drum_sample.hpp"
#include "../instrument.hpp"
#include "../rand_hit.hpp"
#include "../section.hpp"
#include "../song.hpp"
#include "../tempo.hpp"
#include "../time_signature.hpp"
#include "../volume.hpp"

namespace securepath::drum {

template<typename Ar>
Ar& serialise(Ar& ar, beat_hit_data& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.volume & d.accent & d.rand_hit_offset & d.rand_volume;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, beat_hit_data const& d) { return serialise(ar, const_cast<beat_hit_data&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, beat_stop_data& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.falloff;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, beat_stop_data const& d) { return serialise(ar, const_cast<beat_stop_data&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, beat& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.action & d.division & d.hit_data & d.stop_data;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, beat const& d) { return serialise(ar, const_cast<beat&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, bar& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.beats;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, bar const& d) { return serialise(ar, const_cast<bar&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, rand_hit_volume& d) {
	serialisation::sequence<Ar> seq(ar);
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, rand_hit_volume const& d) { return serialise(ar, const_cast<rand_hit_volume&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, rand_hit_offset& d) {
	serialisation::sequence<Ar> seq(ar);
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, rand_hit_offset const& d) { return serialise(ar, const_cast<rand_hit_offset&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, section_bar_change& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.timing_change & d.tempo_change & d.tempo_slide_change;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, section_bar_change const& d) { return serialise(ar, const_cast<section_bar_change&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, section& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.name_ & d.length_ & d.tracks_ & d.changes_;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, section const& d) { return serialise(ar, const_cast<section&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, instrument& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.name_ & d.samples_ & d.volume_;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, instrument const& d) { return serialise(ar, const_cast<instrument&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, drum_sample& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.source_file_;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, drum_sample const& d) { return serialise(ar, const_cast<drum_sample&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, song& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.info_ & d.default_time_signature_ & d.default_tempo_
		& d.instruments_ & d.sections_ & d.section_order_ & d.accent_info_
		& d.rand_offset_ & d.rand_volume_ & serialisation::implicit_tag(1, d.tempo_slide_);
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, song const& d) { return serialise(ar, const_cast<song&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, song_metainfo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.name & d.author & d.notes;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, song_metainfo const& d) { return serialise(ar, const_cast<song_metainfo&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, time_signature& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.beats_in_bar_ & d.beat_type_;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, time_signature const& d) { return serialise(ar, const_cast<time_signature&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, track& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.instrument_index_ & d.bars_ & d.volume_slides_;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, track const& d) { return serialise(ar, const_cast<track&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, volume& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.mute & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, volume const& d) { return serialise(ar, const_cast<volume&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, delta_volume& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, delta_volume const& d) { return serialise(ar, const_cast<delta_volume&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, volume_slide& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.begin & d.end & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, volume_slide const& d) { return serialise(ar, const_cast<volume_slide&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, volume_accent& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, volume_accent const& d) { return serialise(ar, const_cast<volume_accent&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, volume_accent_info& d) {
	serialisation::sequence<Ar> seq(ar);
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, volume_accent_info const& d) { return serialise(ar, const_cast<volume_accent_info&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, audio_falloff& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.type & d.pos & d.end;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, audio_falloff const& d) { return serialise(ar, const_cast<audio_falloff&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, tempo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, tempo const& d) { return serialise(ar, const_cast<tempo&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, delta_tempo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, delta_tempo const& d) { return serialise(ar, const_cast<delta_tempo&>(d)); }

template<typename Ar>
Ar& serialise(Ar& ar, tempo_slide& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.begin & d.end & d.value;
	return ar;
}
template<typename Ar> Ar& serialise(Ar& ar, tempo_slide const& d) { return serialise(ar, const_cast<tempo_slide&>(d)); }

}

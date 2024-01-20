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
void serialise(Ar& ar, beat_hit_data& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.volume & d.accent & d.rand_hit_offset & d.rand_volume;
}

template<typename Ar>
void serialise(Ar& ar, beat_stop_data& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.falloff;
}

template<typename Ar>
void serialise(Ar& ar, beat& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.action & d.division & d.hit_data & d.stop_data;
};

template<typename Ar>
void serialise(Ar& ar, bar& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.beats;
}

template<typename Ar>
void serialise(Ar& ar, rand_hit_volume& d) {
	serialisation::sequence<Ar> seq(ar);
}

template<typename Ar>
void serialise(Ar& ar, rand_hit_offset& d) {
	serialisation::sequence<Ar> seq(ar);
}

template<typename Ar>
void serialise(Ar& ar, section_bar_change& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.timing_change & d.tempo_change & d.tempo_slide_change;
}

template<typename Ar>
void serialise(Ar& ar, section& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.length_ & d.tracks_ & d.changes_;
}

template<typename Ar>
void serialise(Ar& ar, instrument& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.name_ & d.samples_ & d.volume_;
}

template<typename Ar>
void serialise(Ar& ar, drum_sample& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.source_file_;
}

template<typename Ar>
void serialise(Ar& ar, song& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.info_ & d.default_time_signature_ & d.default_tempo_
		& d.instruments_ & d.sections_ & d.section_order_ & d.accent_info_
		& d.rand_offset_ & d.rand_volume_ & serialisation::implicit_tag(1, d.tempo_slide_);
}

template<typename Ar>
void serialise(Ar& ar, song_metainfo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.name & d.author & d.notes;
}

template<typename Ar>
void serialise(Ar& ar, time_signature& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.beats_in_bar_ & d.beat_type_;
}

template<typename Ar>
void serialise(Ar& ar, track& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.bars_ & d.volume_slides_;
}

template<typename Ar>
void serialise(Ar& ar, volume& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.mute & d.value;
}

template<typename Ar>
void serialise(Ar& ar, delta_volume& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
}

template<typename Ar>
void serialise(Ar& ar, volume_slide& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.begin & d.end & d.value;
}

template<typename Ar>
void serialise(Ar& ar, volume_accent& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
}

template<typename Ar>
void serialise(Ar& ar, volume_accent_info& d) {
	serialisation::sequence<Ar> seq(ar);
}

template<typename Ar>
void serialise(Ar& ar, audio_falloff& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.type & d.pos & d.end;
}

template<typename Ar>
void serialise(Ar& ar, tempo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
}

template<typename Ar>
void serialise(Ar& ar, delta_tempo& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.value;
}

template<typename Ar>
void serialise(Ar& ar, tempo_slide& d) {
	serialisation::sequence<Ar> seq(ar);
	seq & d.begin & d.end & d.value;
}

}

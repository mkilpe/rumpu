#ifndef SPDRUM_COMMON_SONG_HEADER
#define SPDRUM_COMMON_SONG_HEADER

#include "section.hpp"
#include "rand_hit.hpp"
#include "track.hpp"
#include "time_signature.hpp"

#include <securepath/serialisation/tag.hpp>

#include <cassert>
#include <iterator>
#include <map>
#include <vector>

namespace securepath::drum {

struct song_metainfo {
	std::string name;
	std::string author;
	std::string notes;

	template<typename Ar>
	void serialise(Ar& ar) {

	}
};

class song {
public:
	using sections_type = std::map<std::uint32_t, section>;
	using section_order_type = std::vector<std::uint32_t>;

	song() = default;
	song(song_metainfo info, time_signature, tempo);

	time_signature default_time_signature() const;
	tempo default_tempo() const;

	std::optional<delta_tempo> global_tempo_slide() const;

	sections_type& sections();
	section_order_type const& section_order() const;
	section_order_type& section_order();
	std::vector<instrument> const& instruments() const;
	std::vector<instrument>& instruments();
	volume_accent_info const& accent_rules() const;
	song_metainfo const& meta_info() const;

	void add_instrument(instrument);
	void load_instruments(std::uint32_t sample_rate = 44100);

	section const* find_section(std::uint32_t) const;
	section* find_section(std::uint32_t);
	std::uint32_t add_section(std::optional<section> = std::nullopt);
	void add_section(std::uint32_t id);

	template<typename Ar>
	void serialise(Ar& ar) {
		serialisation::sequence<Ar> seq(ar);
		seq & info_ & default_time_signature_ & default_tempo_
			& instruments_ & sections_ & section_order_ & accent_info_
			& rand_offset_ & rand_volume_ & serialisation::implicit_tag(1, tempo_slide_);
	}
private:
	// information about this song
	song_metainfo info_;

	// the song default time signature (for example the tempo change is calculated if the time signature changes based on this default)
	time_signature default_time_signature_;

	// the song's tempo with the default time signature
	tempo default_tempo_;

	// the instruments used in this song, in the order of the tracks
	std::vector<instrument> instruments_;

	// sections of the song mapped by id
	sections_type sections_;

	// list of section ids that makes the song
	section_order_type section_order_;

	// set if global volume accent is active
	volume_accent_info accent_info_;

	// set if randomising hit offsets
	rand_hit_offset rand_offset_;

	// set if randomising hit volume
	rand_hit_volume rand_volume_;

	// global tempo slide
	std::optional<delta_tempo> tempo_slide_;
};

}

#endif
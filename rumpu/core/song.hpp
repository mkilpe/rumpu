#pragma once

#include "section.hpp"
#include "rand_hit.hpp"
#include "track.hpp"
#include "time_signature.hpp"

#include <cassert>
#include <iterator>
#include <map>
#include <random>
#include <vector>
#include <shared_mutex>

namespace securepath::drum {

struct song_metainfo {
	std::string name;
	std::string author;
	std::string notes;
};

class song {
public:
	using sections_type = std::map<std::uint32_t, section>;
	using section_order_type = std::vector<std::uint32_t>;

	song() = default;
	song(song_metainfo info, time_signature, tempo);
	song(song&&) noexcept;
	song& operator=(song&&) noexcept;

	time_signature default_time_signature() const;
	tempo default_tempo() const;

	std::optional<delta_tempo> global_tempo_slide() const;

	auto& sections(this auto& self) { return self.sections_; }
	auto& section_order(this auto& self) { return self.section_order_; }
	auto& instruments(this auto& self) { return self.instruments_; }
	volume_accent_info const& accent_rules() const;
	song_metainfo const& meta_info() const;

	void set_metainfo(song_metainfo info);
	void set_default_time_signature(time_signature ts);
	void set_default_tempo(tempo t);

	rand_hit_offset const& rand_offset() const;
	rand_hit_volume const& rand_volume() const;
	void set_rand_offset(rand_hit_offset);
	void set_rand_volume(rand_hit_volume);

	void randomise_beats();
	void randomise_beat(beat&);

	std::size_t add_instrument(instrument);
	void remove_instrument(std::size_t index);
	void load_instruments(std::uint32_t sample_rate = 44100, std::filesystem::path const& base_dir = {});

	auto* find_section(this auto& self, std::uint32_t id) {
		auto it = self.sections_.find(id);
		return it != self.sections_.end() ? &it->second : nullptr;
	}
	std::uint32_t add_section(std::optional<section> = std::nullopt);
	void add_section(std::uint32_t id);
	void remove_section(std::uint32_t id);

	template<typename Ar>
	friend Ar& serialise(Ar&, song&);

public:
	mutable std::shared_mutex mutex;

private:
	void populate_default_beats(section&);

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

	// rng for randomisation
	std::mt19937 rng_{std::random_device{}()};
};

}

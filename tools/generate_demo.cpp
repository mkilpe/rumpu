
#include <rumpu/core/song.hpp>
#include <rumpu/core/song_file.hpp>

#include <filesystem>
#include <iostream>

using namespace securepath::drum;

namespace {

std::string sample_path(std::string const& relative) {
    return (std::filesystem::path(SAMPLES_DIR) / relative).string();
}

beat make_hit(float vol = 1.0f) {
    beat b;
    b.action = beat::hit;
    b.hit_data.volume = {false, vol};
    return b;
}

beat make_none() {
    return beat{};
}

bar make_bar(std::vector<beat> beats) {
    return bar{std::move(beats)};
}

// Standard 4/4 rock kick: 1 and 3
void set_kick_verse(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
    }
}

// Kick with extra ghost note on the "and" of 2
void set_kick_chorus(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b3 = make_hit(0.9f);
        // subdivide beat 3 into two eighth notes: hit + ghost kick
        b3.division = {make_hit(0.9f), make_hit(0.5f)};
        bars[i] = make_bar({make_hit(0.95f), make_none(), b3, make_none()});
    }
}

// Kick fill leading into next section
void set_kick_fill(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size() - 1; ++i) {
        bars[i] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
    }
    // last bar: kick on every beat
    auto& last = bars[bars.size() - 1];
    last = make_bar({make_hit(1.0f), make_hit(0.7f), make_hit(0.9f), make_hit(0.75f)});
}

// Snare on 2 and 4
void set_snare_backbeat(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_none(), make_hit(0.9f), make_none(), make_hit(0.85f)});
    }
}

// Snare with ghost notes
void set_snare_groove(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b1 = make_none();
        b1.division = {make_none(), make_hit(0.3f)}; // ghost on "and" of 1
        beat b3 = make_none();
        b3.division = {make_none(), make_hit(0.35f)}; // ghost on "and" of 3
        bars[i] = make_bar({b1, make_hit(0.9f), b3, make_hit(0.9f)});
    }
}

// Snare fill
void set_snare_fill(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size() - 1; ++i) {
        bars[i] = make_bar({make_none(), make_hit(0.9f), make_none(), make_hit(0.85f)});
    }
    // last bar: snare roll - subdivide each beat into 4
    auto& last = bars[bars.size() - 1];
    beat roll;
    roll.division = {make_hit(0.6f), make_hit(0.5f), make_hit(0.7f), make_hit(0.55f)};
    beat roll2;
    roll2.division = {make_hit(0.7f), make_hit(0.6f), make_hit(0.8f), make_hit(0.65f)};
    beat roll3;
    roll3.division = {make_hit(0.8f), make_hit(0.7f), make_hit(0.9f), make_hit(0.75f)};
    beat roll4;
    roll4.division = {make_hit(0.9f), make_hit(0.8f), make_hit(1.0f), make_hit(0.9f)};
    last = make_bar({roll, roll2, roll3, roll4});
}

// Closed hi-hat eighth notes
void set_hihat_eighths(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b1, b2, b3, b4;
        b1.division = {make_hit(0.8f), make_hit(0.5f)};
        b2.division = {make_hit(0.7f), make_hit(0.5f)};
        b3.division = {make_hit(0.8f), make_hit(0.5f)};
        b4.division = {make_hit(0.7f), make_hit(0.5f)};
        bars[i] = make_bar({b1, b2, b3, b4});
    }
}

// Ride pattern for chorus
void set_ride_pattern(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b1, b2, b3, b4;
        b1.division = {make_hit(0.85f), make_hit(0.45f)};
        b2.division = {make_hit(0.7f), make_hit(0.45f)};
        b3.division = {make_hit(0.85f), make_hit(0.45f)};
        b4.division = {make_hit(0.7f), make_hit(0.45f)};
        bars[i] = make_bar({b1, b2, b3, b4});
    }
}

// Crash on beat 1 only
void set_crash_accent(track& t) {
    auto& bars = t.bars();
    bars[0] = make_bar({make_hit(0.95f), make_none(), make_none(), make_none()});
    for (std::size_t i = 1; i < bars.size(); ++i) {
        bars[i] = make_bar({make_none(), make_none(), make_none(), make_none()});
    }
}

// Tom fill pattern
void set_tom_fill(track& t, float vol) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size() - 1; ++i) {
        bars[i] = make_bar({make_none(), make_none(), make_none(), make_none()});
    }
    auto& last = bars[bars.size() - 1];
    last = make_bar({make_hit(vol), make_none(), make_hit(vol * 0.8f), make_none()});
}

// Silent track
void set_silent(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_none(), make_none(), make_none(), make_none()});
    }
}

section build_section(std::string name, std::uint32_t bars, std::size_t num_instruments,
    std::function<void(std::deque<track>&)> setup)
{
    section sec(bars);
    sec.set_name(std::move(name));
    auto& tracks = sec.tracks();
    for (std::size_t i = 0; i < num_instruments; ++i) {
        sec.add_track(i);
    }
    setup(tracks);
    return sec;
}

} // anon namespace

int main() {
    // Instruments: kick, snare, hi-hat closed, ride, crash, high tom, floor tom
    std::string kick_path   = sample_path("Kick/RD_K_1.wav");
    std::string snare_path  = sample_path("Snare/RD_S_1.wav");
    std::string hihat_path  = sample_path("Cymbals/Hi Hat/RD_C_HH_1.wav");
    std::string ride_path   = sample_path("Cymbals/Ride/RD_C_R_1.wav");
    std::string crash_path  = sample_path("Cymbals/Crash/RD_C_C_1.wav");
    std::string hitom_path  = sample_path("Toms/High Tom/RD_T_HT_1.wav");
    std::string flotom_path = sample_path("Toms/Floor Tom/RD_T_FT_1.wav");

    song s({"Demo Song", "Rumpu", "A demo drum beat with verse/chorus structure"}, {4, 4}, {120.0});
    s.set_rand_offset(rand_hit_offset{2.0f});
    s.set_rand_volume(rand_hit_volume{5.0f});

    // indices: 0=kick, 1=snare, 2=hihat, 3=ride, 4=crash, 5=hitom, 6=flotom
    s.add_instrument(instrument(kick_path));
    s.add_instrument(instrument(snare_path));
    s.add_instrument(instrument(hihat_path));
    s.add_instrument(instrument(ride_path));
    s.add_instrument(instrument(crash_path));
    s.add_instrument(instrument(hitom_path));
    s.add_instrument(instrument(flotom_path));

    std::size_t const num_inst = 7;

    // -- Intro (4 bars): hi-hat eighths + kick, no snare --
    auto intro = build_section("Intro", 4, num_inst, [](auto& tracks) {
        set_kick_verse(tracks[0]);
        set_silent(tracks[1]);      // no snare
        set_hihat_eighths(tracks[2]);
        set_silent(tracks[3]);      // no ride
        set_silent(tracks[4]);      // no crash
        set_silent(tracks[5]);      // no hi tom
        set_silent(tracks[6]);      // no floor tom
    });

    auto make_verse = [&]() {
        return build_section("Verse", 8, num_inst, [](auto& tracks) {
            set_kick_verse(tracks[0]);
            set_snare_backbeat(tracks[1]);
            set_hihat_eighths(tracks[2]);
            set_silent(tracks[3]);
            set_silent(tracks[4]);
            set_silent(tracks[5]);
            set_silent(tracks[6]);
        });
    };

    auto make_fill = [&]() {
        return build_section("Fill", 2, num_inst, [](auto& tracks) {
            set_kick_fill(tracks[0]);
            set_snare_fill(tracks[1]);
            set_hihat_eighths(tracks[2]);
            set_silent(tracks[3]);
            set_silent(tracks[4]);
            set_tom_fill(tracks[5], 0.8f);
            set_tom_fill(tracks[6], 0.85f);
        });
    };

    auto make_chorus = [&]() {
        return build_section("Chorus", 8, num_inst, [](auto& tracks) {
            set_kick_chorus(tracks[0]);
            set_snare_groove(tracks[1]);
            set_silent(tracks[2]);        // no hi-hat
            set_ride_pattern(tracks[3]);  // ride
            set_crash_accent(tracks[4]);  // crash on 1 of first bar
            set_silent(tracks[5]);
            set_silent(tracks[6]);
        });
    };

    auto make_big_fill = [&]() {
        return build_section("Big Fill", 2, num_inst, [](auto& tracks) {
            set_kick_fill(tracks[0]);
            set_snare_fill(tracks[1]);
            set_silent(tracks[2]);
            set_ride_pattern(tracks[3]);
            set_crash_accent(tracks[4]);
            set_tom_fill(tracks[5], 0.9f);
            set_tom_fill(tracks[6], 0.9f);
        });
    };

    // -- Bridge (8 bars): tempo change to 100 BPM, half-time feel --
    auto make_bridge = [&]() {
        auto sec = build_section("Bridge", 8, num_inst, [](auto& tracks) {
            // half-time kick: beat 1 only
            auto& kick_bars = tracks[0].bars();
            for (std::size_t i = 0; i < kick_bars.size(); ++i) {
                kick_bars[i] = make_bar({make_hit(0.85f), make_none(), make_none(), make_none()});
            }
            // snare on beat 3 only (half-time)
            auto& snare_bars = tracks[1].bars();
            for (std::size_t i = 0; i < snare_bars.size(); ++i) {
                snare_bars[i] = make_bar({make_none(), make_none(), make_hit(0.9f), make_none()});
            }
            set_hihat_eighths(tracks[2]);
            set_silent(tracks[3]);
            set_crash_accent(tracks[4]);
            set_silent(tracks[5]);
            set_silent(tracks[6]);
        });
        // tempo change on bar 0 of this section
        sec.set_tempo_change(0, tempo{100.0f});
        return sec;
    };

    // -- Post-bridge fill: brings tempo back to 120 --
    auto make_bridge_fill = [&]() {
        auto sec = build_section("Bridge Fill", 2, num_inst, [](auto& tracks) {
            set_kick_fill(tracks[0]);
            set_snare_fill(tracks[1]);
            set_hihat_eighths(tracks[2]);
            set_silent(tracks[3]);
            set_crash_accent(tracks[4]);
            set_tom_fill(tracks[5], 0.9f);
            set_tom_fill(tracks[6], 0.9f);
        });
        sec.set_tempo_change(0, tempo{120.0f});
        return sec;
    };

    // -- Outro (4 bars): fade out, just kick and hi-hat --
    auto outro = build_section("Outro", 4, num_inst, [](auto& tracks) {
        set_kick_verse(tracks[0]);
        set_silent(tracks[1]);
        set_hihat_eighths(tracks[2]);
        set_silent(tracks[3]);
        set_silent(tracks[4]);
        set_silent(tracks[5]);
        set_silent(tracks[6]);
    });

    // Remove the default empty section and add ours
    // song starts with one default section from constructor, but we built with song(metainfo, ts, tempo)
    // which doesn't add sections, so we just add directly

    // Structure: Intro - Verse - Fill - Chorus - Fill - Verse - Fill - Chorus -
    //            Big Fill - Bridge (100 BPM) - Bridge Fill (back to 120) - Chorus - Big Fill - Outro
    auto intro_id       = s.add_section(std::move(intro));
    auto verse1_id      = s.add_section(make_verse());
    auto fill1_id       = s.add_section(make_fill());
    auto chorus1_id     = s.add_section(make_chorus());
    auto fill2_id       = s.add_section(make_fill());
    auto verse2_id      = s.add_section(make_verse());
    auto fill3_id       = s.add_section(make_fill());
    auto chorus2_id     = s.add_section(make_chorus());
    auto bigfill1_id    = s.add_section(make_big_fill());
    auto bridge_id      = s.add_section(make_bridge());
    auto bridgefill_id  = s.add_section(make_bridge_fill());
    auto chorus3_id     = s.add_section(make_chorus());
    auto bigfill2_id    = s.add_section(make_big_fill());
    auto outro_id       = s.add_section(std::move(outro));

    s.section_order() = {
        intro_id, verse1_id, fill1_id, chorus1_id,
        fill2_id, verse2_id, fill3_id, chorus2_id,
        bigfill1_id, bridge_id, bridgefill_id, chorus3_id,
        bigfill2_id, outro_id
    };

    save_song_file("demo.spd", s);
    std::cout << "Generated demo.spd" << std::endl;

    return 0;
}

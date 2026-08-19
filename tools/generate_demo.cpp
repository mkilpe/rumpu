
#include <rumpu/core/song.hpp>
#include <rumpu/core/song_file.hpp>

#include <filesystem>
#include <iostream>

using namespace securepath::drum;
namespace fs = std::filesystem;

namespace {

void copy_sample(std::string const& relative, fs::path const& output_dir) {
    auto src = fs::path(SAMPLES_DIR) / relative;
    auto dst = output_dir / "samples" / relative;
    fs::create_directories(dst.parent_path());
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
}

std::string sample_rel(std::string const& relative) {
    return (fs::path("samples") / relative).string();
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

beat make_choke() {
    beat b;
    b.action = beat::stop;
    b.stop_data.falloff = audio_falloff::create(audio_falloff::immediate);
    return b;
}

bar make_bar(std::vector<beat> beats) {
    return bar{std::move(beats)};
}

bar make_empty_bar() {
    return make_bar({make_none(), make_none(), make_none(), make_none()});
}

// HH eighth note helper: each beat subdivided into two eighth notes
beat make_hh_eighth(float down, float up) {
    beat b;
    b.division = {make_hit(down), make_hit(up)};
    return b;
}

bar make_hh_eighths_bar(float down = 0.8f, float up = 0.5f) {
    return make_bar({
        make_hh_eighth(down, up),
        make_hh_eighth(down * 0.9f, up),
        make_hh_eighth(down, up),
        make_hh_eighth(down * 0.9f, up)
    });
}

// Snare 16th roll beat with crescendo factor
beat make_snare_roll_beat(float base) {
    beat b;
    b.division = {
        make_hit(base * 0.7f), make_hit(base * 0.6f),
        make_hit(base * 0.85f), make_hit(base * 0.75f)
    };
    return b;
}

// --- Pattern helpers ---

void set_silent(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_empty_bar();
    }
}

void set_kick_verse(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
    }
}

void set_snare_backbeat(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_none(), make_hit(0.9f), make_none(), make_hit(0.85f)});
    }
}

void set_hihat_eighths(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_hh_eighths_bar();
    }
}

// Kick with ghost note on &of3
void set_kick_chorus(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b3 = make_hit(0.9f);
        b3.division = {make_hit(0.9f), make_hit(0.5f)};
        bars[i] = make_bar({make_hit(0.95f), make_none(), b3, make_none()});
    }
}

// Snare with ghost notes on &of1 and &of3, hit on 2 and 4
void set_snare_chorus(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        beat b1;
        b1.division = {make_none(), make_hit(0.3f)};
        beat b3;
        b3.division = {make_none(), make_hit(0.35f)};
        bars[i] = make_bar({b1, make_hit(0.9f), b3, make_hit(0.9f)});
    }
}

// Ride eighths for chorus
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

// Crash on beat 1 of bar 0 only
void set_crash_accent(track& t) {
    auto& bars = t.bars();
    bars[0] = make_bar({make_hit(0.95f), make_none(), make_none(), make_none()});
    for (std::size_t i = 1; i < bars.size(); ++i) {
        bars[i] = make_empty_bar();
    }
}

// Clap layers with snare: hit on 2 and 4
void set_clap_backbeat(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        bars[i] = make_bar({make_none(), make_hit(0.7f), make_none(), make_hit(0.65f)});
    }
}

// Open HH on beat 4, choke on beat 1 of next bar (on the open HH track)
void set_open_hh_choke_chorus(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        if (i == 0) {
            // first bar: just open HH on beat 4
            bars[i] = make_bar({make_none(), make_none(), make_none(), make_hit(0.8f)});
        } else {
            // choke on beat 1, then open HH on beat 4
            bars[i] = make_bar({make_choke(), make_none(), make_none(), make_hit(0.8f)});
        }
    }
}

// Open HH choke every other bar (for verse 2)
void set_open_hh_verse2(track& t) {
    auto& bars = t.bars();
    for (std::size_t i = 0; i < bars.size(); ++i) {
        if (i % 2 == 1) {
            // odd bars: open HH on beat 4
            bars[i] = make_bar({make_none(), make_none(), make_none(), make_hit(0.75f)});
        } else if (i > 0 && (i - 1) % 2 == 1) {
            // bar after an odd bar: choke on beat 1
            bars[i] = make_bar({make_choke(), make_none(), make_none(), make_none()});
        } else {
            bars[i] = make_empty_bar();
        }
    }
}

section build_section(std::string name, std::uint32_t bars, std::size_t num_instruments,
    std::function<void(std::deque<track>&)> setup)
{
    section sec(bars);
    sec.set_name(std::move(name));
    for (std::size_t i = 0; i < num_instruments; ++i) {
        sec.add_track(i);
    }
    auto& tracks = sec.tracks();
    setup(tracks);
    return sec;
}

// Instrument indices
constexpr std::size_t I_KICK      = 0;
constexpr std::size_t I_SNARE     = 1;
constexpr std::size_t I_HH_CLOSED = 2;
constexpr std::size_t I_HH_OPEN   = 3;
constexpr std::size_t I_RIDE      = 4;
constexpr std::size_t I_CRASH     = 5;
constexpr std::size_t I_SPLASH    = 6;
constexpr std::size_t I_HI_TOM   = 7;
constexpr std::size_t I_MID_TOM  = 8;
constexpr std::size_t I_FLO_TOM  = 9;
constexpr std::size_t I_CLAP     = 10;
constexpr std::size_t NUM_INST   = 11;

void silence_all(std::deque<track>& tracks) {
    for (auto& t : tracks) {
        set_silent(t);
    }
}

// --- Section builders ---

section make_intro() {
    return build_section("Intro", 4, NUM_INST, [](auto& tracks) {
        silence_all(tracks);
        set_kick_verse(tracks[I_KICK]);
        set_hihat_eighths(tracks[I_HH_CLOSED]);
    });
}

section make_verse1() {
    return build_section("Verse 1", 8, NUM_INST, [](auto& tracks) {
        silence_all(tracks);
        set_kick_verse(tracks[I_KICK]);
        set_snare_backbeat(tracks[I_SNARE]);
        set_hihat_eighths(tracks[I_HH_CLOSED]);
    });
}

section make_fill1() {
    return build_section("Fill 1", 2, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // kick: verse pattern bar 0, all beats bar 1
        auto& kick = tracks[I_KICK].bars();
        kick[0] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
        kick[1] = make_bar({make_hit(1.0f), make_hit(0.7f), make_hit(0.9f), make_hit(0.75f)});

        // snare: backbeat bar 0, 16th roll bar 1
        auto& snare = tracks[I_SNARE].bars();
        snare[0] = make_bar({make_none(), make_hit(0.9f), make_none(), make_hit(0.85f)});
        snare[1] = make_bar({
            make_snare_roll_beat(0.6f), make_snare_roll_beat(0.7f),
            make_snare_roll_beat(0.8f), make_snare_roll_beat(1.0f)
        });

        set_hihat_eighths(tracks[I_HH_CLOSED]);

        // tom descend on bar 1: hi tom beat 1, mid tom beat 2, floor tom beat 3
        tracks[I_HI_TOM].bars()[1] = make_bar({make_hit(0.85f), make_none(), make_none(), make_none()});
        tracks[I_MID_TOM].bars()[1] = make_bar({make_none(), make_hit(0.85f), make_none(), make_none()});
        tracks[I_FLO_TOM].bars()[1] = make_bar({make_none(), make_none(), make_hit(0.9f), make_none()});
    });
}

section make_chorus() {
    return build_section("Chorus", 8, NUM_INST, [](auto& tracks) {
        silence_all(tracks);
        set_kick_chorus(tracks[I_KICK]);
        set_snare_chorus(tracks[I_SNARE]);
        set_ride_pattern(tracks[I_RIDE]);
        set_crash_accent(tracks[I_CRASH]);
        set_clap_backbeat(tracks[I_CLAP]);
        set_open_hh_choke_chorus(tracks[I_HH_OPEN]);
    });
}

section make_fill2() {
    return build_section("Fill 2", 2, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // kick on 1 and 3 bar 0, all beats bar 1
        auto& kick = tracks[I_KICK].bars();
        kick[0] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
        kick[1] = make_bar({make_hit(1.0f), make_hit(0.7f), make_hit(0.9f), make_hit(0.75f)});

        // tom cascade: hi tom bar 0, mid+floor bar 1
        auto& hi = tracks[I_HI_TOM].bars();
        hi[0] = make_bar({make_none(), make_none(), make_hit(0.8f), make_hit(0.85f)});
        auto& mid = tracks[I_MID_TOM].bars();
        mid[1] = make_bar({make_hit(0.85f), make_hit(0.8f), make_none(), make_none()});
        auto& flo = tracks[I_FLO_TOM].bars();
        flo[1] = make_bar({make_none(), make_none(), make_hit(0.9f), make_hit(0.85f)});

        // splash accent on bar 1 beat 1
        tracks[I_SPLASH].bars()[1] = make_bar({make_hit(0.9f), make_none(), make_none(), make_none()});
    });
}

section make_verse2() {
    return build_section("Verse 2", 8, NUM_INST, [](auto& tracks) {
        silence_all(tracks);
        set_kick_verse(tracks[I_KICK]);
        set_snare_backbeat(tracks[I_SNARE]);
        set_hihat_eighths(tracks[I_HH_CLOSED]);
        set_open_hh_verse2(tracks[I_HH_OPEN]);
    });
}

section make_fill3() {
    return build_section("Fill 3", 1, NUM_INST, [](auto& tracks) {
        silence_all(tracks);
        // snare 16ths crescendo
        tracks[I_SNARE].bars()[0] = make_bar({
            make_snare_roll_beat(0.5f), make_snare_roll_beat(0.65f),
            make_snare_roll_beat(0.8f), make_snare_roll_beat(1.0f)
        });
        // kick on 1 and 3
        tracks[I_KICK].bars()[0] = make_bar({make_hit(0.9f), make_none(), make_hit(0.85f), make_none()});
    });
}

section make_bridge() {
    auto sec = build_section("Bridge", 8, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // half-time: kick on 1, snare on 3
        auto& kick = tracks[I_KICK].bars();
        for (std::size_t i = 0; i < kick.size(); ++i) {
            kick[i] = make_bar({make_hit(0.85f), make_none(), make_none(), make_none()});
        }
        auto& snare = tracks[I_SNARE].bars();
        for (std::size_t i = 0; i < snare.size(); ++i) {
            snare[i] = make_bar({make_none(), make_none(), make_hit(0.9f), make_none()});
        }

        set_hihat_eighths(tracks[I_HH_CLOSED]);
        set_crash_accent(tracks[I_CRASH]);
    });
    sec.set_tempo_change(0, tempo{100.0f});
    return sec;
}

section make_bridge_fill() {
    auto sec = build_section("Bridge Fill", 2, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // kick on all beats both bars
        auto& kick = tracks[I_KICK].bars();
        kick[0] = make_bar({make_hit(0.9f), make_hit(0.7f), make_hit(0.85f), make_hit(0.7f)});
        kick[1] = make_bar({make_hit(1.0f), make_hit(0.75f), make_hit(0.9f), make_hit(0.8f)});

        // snare roll both bars
        auto& snare = tracks[I_SNARE].bars();
        snare[0] = make_bar({
            make_snare_roll_beat(0.6f), make_snare_roll_beat(0.65f),
            make_snare_roll_beat(0.7f), make_snare_roll_beat(0.75f)
        });
        snare[1] = make_bar({
            make_snare_roll_beat(0.8f), make_snare_roll_beat(0.85f),
            make_snare_roll_beat(0.9f), make_snare_roll_beat(1.0f)
        });

        // tom descend
        tracks[I_HI_TOM].bars()[0] = make_bar({make_hit(0.85f), make_hit(0.8f), make_none(), make_none()});
        tracks[I_MID_TOM].bars()[0] = make_bar({make_none(), make_none(), make_hit(0.85f), make_hit(0.8f)});
        tracks[I_FLO_TOM].bars()[1] = make_bar({make_hit(0.9f), make_hit(0.85f), make_none(), make_none()});
    });
    sec.set_tempo_change(0, tempo{120.0f});
    return sec;
}

section make_big_fill() {
    return build_section("Big Fill", 2, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // kick on all beats
        auto& kick = tracks[I_KICK].bars();
        kick[0] = make_bar({make_hit(0.95f), make_hit(0.7f), make_hit(0.9f), make_hit(0.75f)});
        kick[1] = make_bar({make_hit(1.0f), make_hit(0.8f), make_hit(0.95f), make_hit(0.85f)});

        // snare roll
        auto& snare = tracks[I_SNARE].bars();
        snare[0] = make_bar({
            make_snare_roll_beat(0.7f), make_snare_roll_beat(0.75f),
            make_snare_roll_beat(0.8f), make_snare_roll_beat(0.85f)
        });
        snare[1] = make_bar({
            make_snare_roll_beat(0.85f), make_snare_roll_beat(0.9f),
            make_snare_roll_beat(0.95f), make_snare_roll_beat(1.0f)
        });

        // all toms
        auto& hi = tracks[I_HI_TOM].bars();
        hi[0] = make_bar({make_hit(0.85f), make_none(), make_hit(0.8f), make_none()});
        hi[1] = make_bar({make_hit(0.9f), make_none(), make_none(), make_none()});

        auto& mid = tracks[I_MID_TOM].bars();
        mid[0] = make_bar({make_none(), make_hit(0.8f), make_none(), make_hit(0.85f)});
        mid[1] = make_bar({make_none(), make_hit(0.85f), make_none(), make_none()});

        auto& flo = tracks[I_FLO_TOM].bars();
        flo[0] = make_bar({make_none(), make_none(), make_hit(0.85f), make_none()});
        flo[1] = make_bar({make_none(), make_none(), make_hit(0.9f), make_hit(0.9f)});

        // crash on bar 0 beat 1, splash on bar 1 beat 1
        tracks[I_CRASH].bars()[0] = make_bar({make_hit(0.95f), make_none(), make_none(), make_none()});
        tracks[I_SPLASH].bars()[1] = make_bar({make_hit(0.9f), make_none(), make_none(), make_none()});
    });
}

section make_outro() {
    return build_section("Outro", 4, NUM_INST, [](auto& tracks) {
        silence_all(tracks);

        // kick + HH fading out per bar
        float vol_steps[] = {0.8f, 0.6f, 0.4f, 0.2f};
        auto& kick = tracks[I_KICK].bars();
        auto& hh = tracks[I_HH_CLOSED].bars();
        for (std::size_t i = 0; i < 4; ++i) {
            float v = vol_steps[i];
            kick[i] = make_bar({make_hit(v), make_none(), make_hit(v * 0.9f), make_none()});
            hh[i] = make_hh_eighths_bar(v, v * 0.6f);
        }
    });
}

} // anon namespace

// Sample paths, in instrument index order 0-10
static std::vector<std::string> const demo_samples = {
    "Kick/RD_K_3.wav",
    "Snare/RD_S_5.wav",
    "Cymbals/Hi Hat/RD_C_HH_2.wav",
    "Cymbals/Hi Hat/RD_C_HH_8.wav",
    "Cymbals/Ride/RD_C_R_3.wav",
    "Cymbals/Crash/RD_C_C_2.wav",
    "Cymbals/Splash/RD_C_S_1.wav",
    "Toms/High Tom/RD_T_HT_2.wav",
    "Toms/Mid Tom/RD_T_MT_2.wav",
    "Toms/Floor Tom/RD_T_FT_2.wav",
    "Claps/RD_C_3.wav",
};

int main(int argc, char* argv[]) {
    fs::path output_dir = argc > 1 ? fs::path(argv[1]) : fs::current_path();
    fs::create_directories(output_dir);

    for (auto const& rel : demo_samples) {
        copy_sample(rel, output_dir);
    }

    song s({"Demo Song", "Rumpu", "Drum fills, tempo changes, and chokes"}, {4, 4}, {120.0});
    s.set_rand_offset(rand_hit_offset{2.0f});
    s.set_rand_volume(rand_hit_volume{5.0f});

    // instruments: 0-10
    for (auto const& rel : demo_samples) {
        s.add_instrument(instrument(sample_rel(rel)));
    }

    auto intro_id       = s.add_section(make_intro());
    auto verse1_id      = s.add_section(make_verse1());
    auto fill1_id       = s.add_section(make_fill1());
    auto chorus1_id     = s.add_section(make_chorus());
    auto fill2_id       = s.add_section(make_fill2());
    auto verse2_id      = s.add_section(make_verse2());
    auto fill3_id       = s.add_section(make_fill3());
    auto bridge_id      = s.add_section(make_bridge());
    auto bridgefill_id  = s.add_section(make_bridge_fill());
    auto chorus2_id     = s.add_section(make_chorus());
    auto bigfill_id     = s.add_section(make_big_fill());
    auto outro_id       = s.add_section(make_outro());

    s.section_order() = {
        intro_id, verse1_id, fill1_id, chorus1_id,
        fill2_id, verse2_id, fill3_id, bridge_id,
        bridgefill_id, chorus2_id, bigfill_id, outro_id
    };

    auto output_file = (output_dir / "demo.spd").string();
    save_song_file(output_file, s);
    std::cout << "Generated " << output_file << std::endl;

    return 0;
}

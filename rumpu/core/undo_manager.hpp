#pragma once

#include <rumpu/core/song.hpp>
#include <chrono>
#include <deque>

namespace securepath::drum {

enum class coalesce_key {
    none,
    beat_property,
    track_volume,
    section_name,
    section_length,
    bar_tempo,
    instrument_edit,
};

class undo_manager {
public:
    /// Take a snapshot of the song before a mutation.
    /// Call this BEFORE the mutation happens.
    void snapshot(song const& s, coalesce_key key = coalesce_key::none);

    /// Undo: restores previous state. Returns true if performed.
    bool undo(song& s);

    /// Redo: restores next state. Returns true if performed.
    bool redo(song& s);

    bool can_undo() const;
    bool can_redo() const;

    void clear();

private:
    static constexpr std::size_t max_depth_{100};
    static constexpr auto coalesce_window_ = std::chrono::milliseconds{500};

    struct entry {
        song state;
        coalesce_key key{};
        std::chrono::steady_clock::time_point timestamp;
    };

    std::deque<entry> undo_stack_;
    std::deque<entry> redo_stack_;
};

}

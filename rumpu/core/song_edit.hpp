#pragma once

#include <rumpu/core/song.hpp>
#include <rumpu/core/undo_manager.hpp>

#include <mutex>
#include <shared_mutex>

namespace securepath::drum {

// RAII guard for mutating a song from the UI while the audio thread renders it.
// The mixer reads the song under a shared lock, so every mutation must hold the
// song's exclusive write lock. Constructing a song_edit records an undo snapshot
// (when an undo_manager is supplied) and then takes the write lock for the
// lifetime of the guard. Keep the scope tight so the audio thread is only
// briefly blocked. Never construct one while already holding the player lock:
// the audio thread takes the player lock and then the song lock, so the reverse
// order would deadlock.
class song_edit {
public:
	explicit song_edit(song& s, undo_manager* undo = nullptr, coalesce_key key = coalesce_key::none)
	: song_(s)
	{
		// snapshot is a read-only deep copy; take it before locking so the audio
		// thread is not blocked for the duration of the copy
		if(undo) {
			undo->snapshot(s, key);
		}
		lock_ = std::unique_lock<std::shared_mutex>(s.mutex);
	}

	song& get() {
		return song_;
	}
	song* operator->() {
		return &song_;
	}
	song& operator*() {
		return song_;
	}

private:
	song& song_;
	std::unique_lock<std::shared_mutex> lock_{};
};

}


#include "undo_manager.hpp"

namespace securepath::drum::app {

void undo_manager::snapshot(song const& s, coalesce_key key) {
    auto now = std::chrono::steady_clock::now();

    // Coalesce: if same non-none key within the time window, replace top
    if (key != coalesce_key::none && !undo_stack_.empty()) {
        auto& top = undo_stack_.back();
        if (top.key == key && (now - top.timestamp) < coalesce_window_) {
            // Keep the original pre-mutation state, just update timestamp
            top.timestamp = now;
            redo_stack_.clear();
            return;
        }
    }

    undo_stack_.push_back(entry{song{s}, key, now});
    redo_stack_.clear();

    if (undo_stack_.size() > max_depth_) {
        undo_stack_.pop_front();
    }
}

bool undo_manager::undo(song& s) {
    if (undo_stack_.empty()) {
        return false;
    }

    redo_stack_.push_back(entry{song{s}, coalesce_key::none, std::chrono::steady_clock::now()});

    auto& top = undo_stack_.back();
    s = std::move(top.state);
    undo_stack_.pop_back();
    return true;
}

bool undo_manager::redo(song& s) {
    if (redo_stack_.empty()) {
        return false;
    }

    undo_stack_.push_back(entry{song{s}, coalesce_key::none, std::chrono::steady_clock::now()});

    auto& top = redo_stack_.back();
    s = std::move(top.state);
    redo_stack_.pop_back();
    return true;
}

bool undo_manager::can_undo() const {
    return !undo_stack_.empty();
}

bool undo_manager::can_redo() const {
    return !redo_stack_.empty();
}

void undo_manager::clear() {
    undo_stack_.clear();
    redo_stack_.clear();
}

}

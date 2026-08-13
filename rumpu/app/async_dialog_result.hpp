#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace securepath::drum::app {

// Thread-safe single-value mailbox shared between the UI and a native file
// dialog thread. The dialog callback must capture a shared_ptr to this (never
// the owning object), so a result delivered after the owner is gone lands in
// the mailbox and is dropped with it.
class async_dialog_result {
public:
    // Marks a dialog in flight. Returns false if one is already running.
    bool begin() {
        std::lock_guard l{mutex_};
        if (in_flight_) {
            return false;
        }
        in_flight_ = true;
        return true;
    }

    // Called from the dialog thread; empty value means cancelled.
    void deliver(std::string value) {
        std::lock_guard l{mutex_};
        value_ = std::move(value);
        has_value_ = true;
    }

    // Called from the UI thread. Returns the delivered value at most once
    // (empty string = cancelled) and clears the in-flight state.
    std::optional<std::string> take() {
        std::lock_guard l{mutex_};
        if (!has_value_) {
            return std::nullopt;
        }
        has_value_ = false;
        in_flight_ = false;
        return std::move(value_);
    }

    bool in_flight() const {
        std::lock_guard l{mutex_};
        return in_flight_;
    }

private:
    mutable std::mutex mutex_;
    std::string value_;
    bool has_value_{};
    bool in_flight_{};
};

using async_dialog_result_ptr = std::shared_ptr<async_dialog_result>;

}

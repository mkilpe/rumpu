#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace securepath::drum::app {

// Thread-safe single-value mailbox shared between the UI and a native file
// dialog thread. The dialog callback must capture a shared_ptr to this (never
// the owning object), so a result delivered after the owner is gone lands in
// the mailbox and is dropped with it.
//
// Deliveries are tied to a session: begin() hands out a token the callback
// must pass to deliver(), and invalidate() (called when the owning dialog
// reopens) makes older tokens no-ops. Without this, a chooser left open across
// a dialog close/reopen would deliver into the new session and the value would
// be applied to whatever is selected then.
class async_dialog_result {
public:
    // Marks a dialog in flight and returns the token for its delivery.
    // Returns nullopt if one is already running.
    std::optional<std::uint64_t> begin() {
        std::lock_guard l{mutex_};
        if (in_flight_) {
            return std::nullopt;
        }
        in_flight_ = true;
        return session_;
    }

    // Called from the dialog thread; empty value means cancelled. A delivery
    // whose session has been invalidated is dropped.
    void deliver(std::uint64_t session, std::string value) {
        std::lock_guard l{mutex_};
        if (session == session_) {
            value_ = std::move(value);
            has_value_ = true;
        }
    }

    // Called from the UI thread when the owning dialog (re)opens: drops any
    // stored value and detaches any chooser still open from an earlier session.
    void invalidate() {
        std::lock_guard l{mutex_};
        ++session_;
        in_flight_ = false;
        has_value_ = false;
        value_.clear();
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
    std::uint64_t session_{};
    bool has_value_{};
    bool in_flight_{};
};

using async_dialog_result_ptr = std::shared_ptr<async_dialog_result>;

}

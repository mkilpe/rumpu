#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

namespace securepath::drum::app {

// Thread-safe single-value mailbox shared between the UI and a worker thread
// (a native file dialog, a folder scan...). The worker callback must capture a
// shared_ptr to this (never the owning object), so a result delivered after
// the owner is gone lands in the mailbox and is dropped with it.
//
// Deliveries are tied to a session: begin() hands out a token the worker
// must pass to deliver(), and invalidate() (called when the owning dialog
// reopens or the request is superseded) makes older tokens no-ops. Without
// this, a worker left running across a dialog close/reopen would deliver into
// the new session and the value would be applied to whatever is current then.
template<typename T>
class async_result {
public:
    // Marks a request in flight and returns the token for its delivery.
    // Returns nullopt if one is already running.
    std::optional<std::uint64_t> begin() {
        std::lock_guard l{mutex_};
        if (in_flight_) {
            return std::nullopt;
        }
        in_flight_ = true;
        return session_;
    }

    // Called from the worker thread. A delivery whose session has been
    // invalidated is dropped.
    void deliver(std::uint64_t session, T value) {
        std::lock_guard l{mutex_};
        if (session == session_) {
            value_ = std::move(value);
            has_value_ = true;
        }
    }

    // Called from the UI thread when the owning dialog (re)opens or the
    // request is superseded: drops any stored value and detaches any worker
    // still running from an earlier session.
    void invalidate() {
        std::lock_guard l{mutex_};
        ++session_;
        in_flight_ = false;
        has_value_ = false;
        value_ = T{};
    }

    // Called from the UI thread. Returns the delivered value at most once
    // and clears the in-flight state.
    std::optional<T> take() {
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
    T value_{};
    std::uint64_t session_{};
    bool has_value_{};
    bool in_flight_{};
};

// native file dialogs deliver the chosen path (empty string = cancelled)
using async_dialog_result = async_result<std::string>;
using async_dialog_result_ptr = std::shared_ptr<async_dialog_result>;

// Begins a session on the mailbox and opens the native dialog with a callback
// delivering under that session's token, so no call site can forget the token
// and reintroduce stale cross-session deliveries. Returns false (and does not
// open) when a chooser is already in flight.
template<typename OpenDialogFn>
bool launch_dialog(async_dialog_result_ptr const& result, OpenDialogFn&& open) {
    auto session = result->begin();
    if (!session) {
        return false;
    }
    open([r = result, s = *session](std::string p) {
        r->deliver(s, std::move(p));
    });
    return true;
}

}

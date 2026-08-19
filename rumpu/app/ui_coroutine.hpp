#pragma once

#include <securepath/log/log.hpp>

#include <coroutine>
#include <exception>
#include <utility>

namespace securepath::drum::app {

/// Awaiter that suspends the coroutine until the next frame.
struct next_frame {
    bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<>) const noexcept {}
    void await_resume() const noexcept {}
};

/// Coroutine return type for per-frame ImGui coroutines.
///
/// Call tick() once per frame to resume the coroutine. The coroutine
/// draws its ImGui content, then co_awaits next_frame{} to yield
/// back to the caller. When the coroutine co_returns, tick() returns
/// false and active() becomes false.
class ui_task {
public:
    struct promise_type {
        ui_task get_return_object() {
            return ui_task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }

        std::exception_ptr exception;
    };

    ui_task() = default;

    explicit ui_task(std::coroutine_handle<promise_type> h)
    : handle_(h) {}

    ui_task(ui_task const&) = delete;
    ui_task& operator=(ui_task const&) = delete;

    ui_task(ui_task&& other) noexcept
    : handle_(std::exchange(other.handle_, {})) {}

    ui_task& operator=(ui_task&& other) noexcept {
        if (this != &other) {
            destroy();
            handle_ = std::exchange(other.handle_, {});
        }
        return *this;
    }

    ~ui_task() { destroy(); }

    /// Resume the coroutine for one frame. Returns true if still active.
    /// An exception escaping the coroutine ends it (the dialog closes) and is
    /// logged rather than silently discarded; it is not rethrown because the
    /// ImGui window stack is already unbalanced at that point.
    bool tick() {
        if (!handle_ || handle_.done()) {
            return false;
        }
        handle_.resume();
        if (auto ex = std::exchange(handle_.promise().exception, {})) {
            try {
                std::rethrow_exception(ex);
            } catch (std::exception const& e) {
                LOG_WARN("ui coroutine ended with exception: {}", e.what());
            } catch (...) {
                LOG_WARN("ui coroutine ended with unknown exception");
            }
        }
        return !handle_.done();
    }

    bool active() const {
        return handle_ && !handle_.done();
    }

    explicit operator bool() const { return active(); }

private:
    void destroy() {
        if (handle_) {
            handle_.destroy();
            handle_ = {};
        }
    }

    std::coroutine_handle<promise_type> handle_;
};

} // namespace securepath::drum::app

#include <catch2/catch_all.hpp>

#include <rumpu/app/ui_coroutine.hpp>

#include <stdexcept>

using namespace securepath::drum::app;

static ui_task counting_task(int& frames) {
	++frames;
	co_await next_frame{};
	++frames;
}

static ui_task throwing_task(int& frames) {
	++frames;
	co_await next_frame{};
	throw std::runtime_error("dialog blew up");
}

TEST_CASE("ui_task runs one step per tick and finishes", "[ui_task]") {
	int frames = 0;
	ui_task t = counting_task(frames);
	CHECK(t.active());
	CHECK(t.tick());
	CHECK(frames == 1);
	CHECK(!t.tick()); // second resume runs to completion
	CHECK(frames == 2);
	CHECK(!t.active());
}

TEST_CASE("ui_task ends cleanly when the coroutine throws", "[ui_task]") {
	// an escaping exception must end the task (dialog closes), not crash or
	// silently leave it resumable
	int frames = 0;
	ui_task t = throwing_task(frames);
	CHECK(t.tick());
	CHECK_NOTHROW(t.tick());
	CHECK(!t.active());
	CHECK(frames == 1);
}

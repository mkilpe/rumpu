#include <catch2/catch_all.hpp>

#include <rumpu/app/async_dialog_result.hpp>

#include <thread>

using securepath::drum::app::async_dialog_result;

TEST_CASE("mailbox delivers a value once", "[async_dialog_result]") {
	async_dialog_result r;
	CHECK(!r.take());
	auto session = r.begin();
	REQUIRE(session);
	CHECK(r.in_flight());
	r.deliver(*session, "/tmp/file.wav");

	auto v = r.take();
	REQUIRE(v);
	CHECK(*v == "/tmp/file.wav");
	CHECK(!r.in_flight());
	CHECK(!r.take());
}

TEST_CASE("only one dialog can be in flight", "[async_dialog_result]") {
	async_dialog_result r;
	auto session = r.begin();
	REQUIRE(session);
	CHECK(!r.begin());
	r.deliver(*session, "x");
	REQUIRE(r.take());
	CHECK(r.begin());
}

TEST_CASE("cancel (empty result) clears in-flight state", "[async_dialog_result]") {
	async_dialog_result r;
	auto session = r.begin();
	REQUIRE(session);
	r.deliver(*session, "");

	auto v = r.take();
	REQUIRE(v);
	CHECK(v->empty());
	CHECK(!r.in_flight());
	CHECK(r.begin());
}

TEST_CASE("delivery from another thread is visible", "[async_dialog_result]") {
	async_dialog_result r;
	auto session = r.begin();
	REQUIRE(session);
	std::thread t{[&r, s = *session] { r.deliver(s, "threaded"); }};
	t.join();

	auto v = r.take();
	REQUIRE(v);
	CHECK(*v == "threaded");
}

TEST_CASE("invalidate drops a delivery from a previous session", "[async_dialog_result]") {
	async_dialog_result r;
	auto session = r.begin();
	REQUIRE(session);

	r.invalidate(); // the owning dialog was closed and reopened
	r.deliver(*session, "stale.wav"); // the old chooser finally returns

	CHECK(!r.take());
	CHECK(!r.in_flight());
}

TEST_CASE("invalidate drops an already-delivered value", "[async_dialog_result]") {
	async_dialog_result r;
	auto session = r.begin();
	REQUIRE(session);
	r.deliver(*session, "old.wav"); // delivered while the dialog was closed

	r.invalidate();
	CHECK(!r.take());
}

TEST_CASE("current session still delivers after an old one is dropped", "[async_dialog_result]") {
	async_dialog_result r;
	auto old_session = r.begin();
	REQUIRE(old_session);

	r.invalidate();
	auto session = r.begin();
	REQUIRE(session);

	r.deliver(*old_session, "stale.wav");
	CHECK(!r.take());
	CHECK(r.in_flight());

	r.deliver(*session, "fresh.wav");
	auto v = r.take();
	REQUIRE(v);
	CHECK(*v == "fresh.wav");
}

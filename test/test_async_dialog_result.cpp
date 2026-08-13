#include <catch2/catch_all.hpp>

#include <rumpu/app/async_dialog_result.hpp>

#include <thread>

using securepath::drum::app::async_dialog_result;

TEST_CASE("mailbox delivers a value once", "[async_dialog_result]") {
	async_dialog_result r;
	CHECK(!r.take());
	REQUIRE(r.begin());
	CHECK(r.in_flight());
	r.deliver("/tmp/file.wav");

	auto v = r.take();
	REQUIRE(v);
	CHECK(*v == "/tmp/file.wav");
	CHECK(!r.in_flight());
	CHECK(!r.take());
}

TEST_CASE("only one dialog can be in flight", "[async_dialog_result]") {
	async_dialog_result r;
	REQUIRE(r.begin());
	CHECK(!r.begin());
	r.deliver("x");
	REQUIRE(r.take());
	CHECK(r.begin());
}

TEST_CASE("cancel (empty result) clears in-flight state", "[async_dialog_result]") {
	async_dialog_result r;
	REQUIRE(r.begin());
	r.deliver("");

	auto v = r.take();
	REQUIRE(v);
	CHECK(v->empty());
	CHECK(!r.in_flight());
	CHECK(r.begin());
}

TEST_CASE("delivery from another thread is visible", "[async_dialog_result]") {
	async_dialog_result r;
	REQUIRE(r.begin());
	std::thread t{[&r] { r.deliver("threaded"); }};
	t.join();

	auto v = r.take();
	REQUIRE(v);
	CHECK(*v == "threaded");
}

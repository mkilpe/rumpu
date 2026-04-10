#include <catch2/catch_all.hpp>

#include <rumpu/core/song.hpp>
#include <rumpu/core/undo_manager.hpp>

#include <thread>

using namespace securepath::drum;

namespace {

song make_song(float bpm = 120.0f) {
	return song{{}, {4, 4}, {bpm}};
}

}

TEST_CASE("undo_manager starts empty", "[undo]") {
	undo_manager um;
	CHECK(!um.can_undo());
	CHECK(!um.can_redo());
}

TEST_CASE("undo on empty stack returns false and leaves song untouched", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);
	CHECK(!um.undo(s));
	CHECK(!um.redo(s));
	CHECK(s.default_tempo().value == 120.0f);
}

TEST_CASE("undo restores pre-snapshot tempo", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s);
	s.set_default_tempo(tempo{140.0f});
	REQUIRE(s.default_tempo().value == 140.0f);

	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 120.0f);
	CHECK(um.can_redo());
	CHECK(!um.can_undo());
}

TEST_CASE("redo reapplies undone mutation", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s);
	s.set_default_tempo(tempo{140.0f});

	REQUIRE(um.undo(s));
	REQUIRE(s.default_tempo().value == 120.0f);

	REQUIRE(um.redo(s));
	CHECK(s.default_tempo().value == 140.0f);
	CHECK(um.can_undo());
	CHECK(!um.can_redo());
}

TEST_CASE("new snapshot clears redo stack", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s);
	s.set_default_tempo(tempo{140.0f});
	REQUIRE(um.undo(s));
	REQUIRE(um.can_redo());

	um.snapshot(s);
	CHECK(!um.can_redo());
}

TEST_CASE("clear empties both stacks", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s);
	s.set_default_tempo(tempo{140.0f});
	um.undo(s);
	REQUIRE(um.can_redo());

	um.clear();
	CHECK(!um.can_undo());
	CHECK(!um.can_redo());
}

TEST_CASE("same coalesce key within window replaces top entry", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s, coalesce_key::bar_tempo);
	s.set_default_tempo(tempo{130.0f});
	um.snapshot(s, coalesce_key::bar_tempo);
	s.set_default_tempo(tempo{140.0f});

	REQUIRE(um.undo(s));
	// Coalesced: single undo should jump all the way back to the pre-first-snapshot state
	CHECK(s.default_tempo().value == 120.0f);
	CHECK(!um.can_undo());
}

TEST_CASE("coalesce_key::none never coalesces", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s, coalesce_key::none);
	s.set_default_tempo(tempo{130.0f});
	um.snapshot(s, coalesce_key::none);
	s.set_default_tempo(tempo{140.0f});

	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 130.0f);
	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 120.0f);
}

TEST_CASE("different coalesce keys do not coalesce", "[undo]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s, coalesce_key::bar_tempo);
	s.set_default_tempo(tempo{130.0f});
	um.snapshot(s, coalesce_key::track_volume);
	s.set_default_tempo(tempo{140.0f});

	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 130.0f);
	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 120.0f);
}

TEST_CASE("same coalesce key outside window keeps both entries", "[undo][.slow]") {
	undo_manager um;
	auto s = make_song(120.0f);

	um.snapshot(s, coalesce_key::bar_tempo);
	s.set_default_tempo(tempo{130.0f});

	std::this_thread::sleep_for(std::chrono::milliseconds{600});

	um.snapshot(s, coalesce_key::bar_tempo);
	s.set_default_tempo(tempo{140.0f});

	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 130.0f);
	REQUIRE(um.undo(s));
	CHECK(s.default_tempo().value == 120.0f);
}

TEST_CASE("undo stack is capped at max depth", "[undo]") {
	undo_manager um;
	auto s = make_song(0.0f);

	// Push 101 snapshots with distinct tempos (1, 2, 3, ... 101).
	// With max_depth=100, the oldest entry (tempo 0.0, the original) should be dropped.
	for (int i = 1; i <= 101; ++i) {
		um.snapshot(s);
		s.set_default_tempo(tempo{static_cast<float>(i)});
	}

	int undo_count{};
	while (um.undo(s)) {
		++undo_count;
	}
	CHECK(undo_count == 100);
	// The oldest surviving snapshot was taken after setting tempo to 1.0,
	// so after undoing everything we land on 1.0, not 0.0.
	CHECK(s.default_tempo().value == 1.0f);
}

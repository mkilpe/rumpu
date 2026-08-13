#include <catch2/catch_all.hpp>

#include <rumpu/core/song.hpp>

using namespace securepath::drum;

TEST_CASE("add_section does not reuse a live section id", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	s.find_section(id2)->set_length(7);

	s.remove_section(id1);
	auto id3 = s.add_section();

	CHECK(id3 != id2);
	REQUIRE(s.find_section(id2) != nullptr);
	CHECK(s.find_section(id2)->length() == 7);
	REQUIRE(s.find_section(id3) != nullptr);
}

TEST_CASE("add_section after removal clones layout from surviving section", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	auto id2 = s.add_section();
	auto& sec2 = *s.find_section(id2);
	sec2.add_track(0);
	sec2.add_track(1);

	s.remove_section(id1);
	auto id3 = s.add_section();

	REQUIRE(s.find_section(id3) != nullptr);
	CHECK(s.find_section(id3)->tracks().size() == sec2.tracks().size());
	CHECK(s.find_section(id2)->tracks().size() == 2);
}

TEST_CASE("add_section starts from id 1 on an empty song", "[song]") {
	song s{{}, {4, 4}, {120}};
	auto id1 = s.add_section();
	CHECK(id1 == 1);
	s.remove_section(id1);
	CHECK(s.add_section() == 1);
}

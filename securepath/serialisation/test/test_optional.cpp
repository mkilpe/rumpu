#include <securepath/test_frame/test_suite.hpp>
#include <securepath/test_frame/test_utils.hpp>

#include "util.hpp"
#include "types.hpp"

namespace securepath::serialisation::test {

struct containee {
	std::uint16_t v = 0;

	template<typename S>
	void serialise(S& s) {
		serialisation::sequence<S> seq(s);
		seq & v;
	}

	bool operator==(containee const c) const {
		return v == c.v;
	}
};

struct container {

	std::uint16_t v;
	std::optional<containee> opt;

	template<typename S>
	void serialise(S& s) {
		serialisation::sequence<S> seq(s);
		seq & v & opt;
	}

	bool operator==(container const c) const {
		return v == c.v && opt == c.opt;
	}
};

TEST_CASE("optional composite test", "[serialisation][optional]") {
	std::vector<container> vec(2);
	CHECK(check(vec));
}

}

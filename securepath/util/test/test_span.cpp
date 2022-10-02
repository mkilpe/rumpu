#include <securepath/test_frame/test_suite.hpp>
#include <securepath/test_frame/test_utils.hpp>
#include <securepath/util/span.hpp>
#include <securepath/util/types.hpp>

namespace securepath::test {

TEST_CASE("span octet span basic", "[span]") {

	octet_vector o = securepath::test::random_octet_vector((std::size_t)10);
	octet_span os(o.data(), o.size());

	CHECK(!os.empty());
	CHECK(os.size() == o.size());

	auto d = os.data();
	CHECK(d == o.data());

	octet_vector res(os.begin(), os.end());
	CHECK(o == res);

}

}

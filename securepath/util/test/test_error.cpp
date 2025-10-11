#include <securepath/test_frame/test_suite.hpp>
#include <securepath/util/error.hpp>

#include <sstream>

namespace securepath::test {

TEST_CASE("error basics", "[error]") {
	
	errc e_code = errc::exception_occurred;
	std::error_code err_code = make_error_code(e_code);

	error err(err_code, "error_msg");

	CHECK(err);
	CHECK(err.message() == "error_msg");
	CHECK((int)err.code().value() == (int)e_code);
	CHECK(std::string(err.code().category().name()) == "securepath error");

	std::ostringstream os;
	std::print(os, "{}", err);
	std::string res = os.str();

	CHECK(res.find("error_msg") != std::string::npos);
	CHECK(res.find("") != std::string::npos);
	CHECK(res.find("securepath error") != std::string::npos);
	CHECK(res.find(std::to_string((int)err.code().value())) != std::string::npos);
}

}
